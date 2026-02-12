#include "stdafx.h"
#include "TestScene.h"

#include "InputManager.h"
#include "SceneManager.h"
#include "TimeManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "CameraComponent.h"
#include "GameManager.h"
#include "ColliderComponent.h"

#include "TestCameraObject.h"
#include "CamRotObject.h"

#include "Player.h"
#include "Enemy.h"
#include "RNG.h"

#include "Panel.h"
#include "Button.h"

#include "Shared/Config/Option.h"
#include <Slider.h>
#include "Text.h"

REGISTER_TYPE(TestScene)

using namespace std;
using namespace DirectX;

void TestScene::Initialize()
{
	GameManager::GetInstance().OnSceneEnter(EScene::Main);
	GameManager::GetInstance().ForceShowCursor(FALSE);

	m_player = dynamic_cast<Player*>(GetRootGameObject("Player"));

	m_tutorialBox = GetRootGameObject("Box");
	m_stage2Trigger = GetRootGameObject("Stage2Trigger");
	m_stageBossTrigger = GetRootGameObject("StageBossTrigger");
}

void TestScene::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();
	SpawnEnemy(deltaTime);

	GameManager::GetInstance().OnSceneUpdate();
	TutorialStep();

	CheckStageTrigger();
	SetScoreUI(GameManager::GetInstance().GetScore());
	if (m_player)
	{
		UpdateHPUI(m_player->GetHPRatio());
		UpdateDeadEyeUI(m_player->GetDeadEyeCooldownRatio());
		UpdateBulletUI(m_player->GetBulletCount());
	}
}

void TestScene::Render()
{
	GameManager::GetInstance().OnSceneRender();

	//RenderSpawnPoints();
}

#ifdef _DEBUG
void TestScene::RenderImGui()
{
	if (ImGui::TreeNode("Spawn Points"))
	{
		for (XMVECTOR& point : m_spawnPoints) ImGui::DragFloat3(("Point " + to_string(&point - &m_spawnPoints[0])).c_str(), &point.m128_f32[0], 0.1f);
		if (ImGui::Button("Add Point")) m_spawnPoints.push_back(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
		if (!m_spawnPoints.empty() && ImGui::Button("Remove Last Point")) m_spawnPoints.pop_back();

		ImGui::TreePop();
	}
}
#endif

void TestScene::Finalize()
{
	GameManager::GetInstance().OnSceneExit();
}

nlohmann::json TestScene::Serialize()
{
	nlohmann::json jsonData = {};
	nlohmann::json spawnPointsData = nlohmann::json::array();

	for (const XMVECTOR& point : m_spawnPoints) spawnPointsData.push_back({ XMVectorGetX(point), XMVectorGetY(point), XMVectorGetZ(point) });
	jsonData["spawnPoints"] = spawnPointsData;

	return jsonData;
}

void TestScene::Deserialize(const nlohmann::json& jsonData)
{
	m_spawnPoints.clear();
	if (jsonData.find("spawnPoints") != jsonData.end())
	{
		const nlohmann::json& spawnPointsData = jsonData["spawnPoints"];
		for (const auto& pointData : spawnPointsData)
		{
			XMVECTOR point = XMVectorSet
			(
				pointData[0].get<float>(),
				pointData[1].get<float>(),
				pointData[2].get<float>(),
				1.0f
			);
			m_spawnPoints.emplace_back(point);
		}
	}
}

void TestScene::TutorialStep()
{
	switch (GameManager::GetInstance().GetTutorialStep())
	{
	case ETutorialStep::WASD:
		constexpr float BOX_DISTANCE_SQ = 8.0f;
		if (m_tutorialBox && XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(m_player->GetWorldPosition(), m_tutorialBox->GetWorldPosition()))) < BOX_DISTANCE_SQ)
		{
			m_tutorialBox->SetAlive(false);
			GameManager::GetInstance().SetTutorialStep(ETutorialStep::Dash);
		}
		break;
	}
}

void TestScene::CheckStageTrigger()
{
	const XMVECTOR& playerPos = m_player->GetWorldPosition();
	if (m_stage2Trigger && m_stage2Trigger->GetComponent<ColliderComponent>()->CheckCollisionPoint(XMVectorSetY(playerPos, 0.0f)))
	{
		GameManager::GetInstance().ChangeMainState(EMainState::Stage2);
		m_stage2Trigger->SetAlive(false);
		m_stage2Trigger = nullptr;
	}
	if (m_stageBossTrigger && m_stageBossTrigger->GetComponent<ColliderComponent>()->CheckCollisionPoint(XMVectorSetY(playerPos, 0.0f)))
	{
		GameManager::GetInstance().ChangeMainState(EMainState::StageBoss);
		m_stageBossTrigger->SetAlive(false);
		m_stageBossTrigger = nullptr;
	}
}

// 스폰 활성 거리 제곱
constexpr float SPAWN_ACTIVE_DISTANCE_SQ = 25.0f * 25.0f;

void TestScene::SpawnEnemy(float deltaTime)
{
	static float spawnTime = 0.0f;
	spawnTime += deltaTime;

	if (spawnTime > m_spawnInterval)
	{
		vector<XMVECTOR> validSpawnPoints = {};
		copy_if
		(
			m_spawnPoints.begin(),
			m_spawnPoints.end(),
			back_inserter(validSpawnPoints),
			[&](const XMVECTOR& point) { return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(GetRootGameObject("Player")->GetWorldPosition(), point))) < SPAWN_ACTIVE_DISTANCE_SQ; }
		);

		if (!validSpawnPoints.empty())
		{
			for (const XMVECTOR& spawnPoint : validSpawnPoints) CreatePrefabRootGameObject("Enemy.json")->SetPosition(spawnPoint);

			spawnTime = 0.0f;
		}
	}
}

void TestScene::RenderSpawnPoints()
{
	Renderer::GetInstance().UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			pair<com_ptr<ID3D11ShaderResourceView>, XMFLOAT2> spawnPointTextureAndOffset = {};
			spawnPointTextureAndOffset = ResourceManager::GetInstance().GetTextureAndOffset("Crosshair.png");

			vector<XMVECTOR> insideViewFrustumSpawnPoints = {};
			copy_if
			(
				m_spawnPoints.begin(),
				m_spawnPoints.end(),
				back_inserter(insideViewFrustumSpawnPoints),
				[](const XMVECTOR& point)
				{
					BoundingFrustum frustum = CameraComponent::GetMainCamera().GetBoundingFrustum();
					return frustum.Contains(point) != DirectX::DISJOINT;
				}
			);

			for (const XMVECTOR& point : insideViewFrustumSpawnPoints)
			{
				Renderer::GetInstance().RenderImageScreenPosition
				(
					spawnPointTextureAndOffset.first,
					CameraComponent::GetMainCamera().WorldToScreenPosition(point),
					spawnPointTextureAndOffset.second,
					0.5f,
					XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(GetRootGameObject("Player")->GetWorldPosition(), point))) > SPAWN_ACTIVE_DISTANCE_SQ ? XMVECTOR{ 1.0f, 0.0f, 0.0f, 1.0f } : XMVECTOR{ 0.0f, 1.0f, 0.0f, 1.0f }
				);
			}
		}
	);
}

void TestScene::BindUIActions()
{
	for (const auto& uiPtr : m_UIList) {
		if (auto* panel = dynamic_cast<Panel*>(uiPtr.get())) {
			if (panel->GetName() == "option") optionPanel = panel;
			else if (panel->GetName() == "1") n1 = panel;
			else if (panel->GetName() == "10") n10 = panel;
			else if (panel->GetName() == "100") n100 = panel;
			else if (panel->GetName() == "1000") n1000 = panel;
			else if (panel->GetName() == "10000") n10000 = panel;
			else if (panel->GetName() == "100000") n100000 = panel;
			else if (panel->GetName() == "combo") combo = panel;
			else if (panel->GetName() == "IngameUI") IngameUI = panel;
			else if (panel->GetName() == "HP_Bar") hpBar = panel;
			else if (panel->GetName() == "HP_Deco") hpDeco = panel;
			else if (panel->GetName() == "DeadEye") deadEye = panel;
			else if (panel->GetName() == "bullet") bullet = panel;
		}
	}
	if (optionPanel)
	{
		GameManager::GetInstance().RegisterOptionPanel(optionPanel);
	}


	for (auto& uiPtr : m_UIList) {
		// -------------------------------------------------------
		// 1. Button bindings
		// -------------------------------------------------------
		if (auto* btn = dynamic_cast<Button*>(uiPtr.get()))
		{
			std::string key = btn->GetActionKey();

			if (key == "close_option") {
				if (optionPanel) btn->SetOnClick([this]()
					{
						optionPanel->SetActive(false);
						GameManager::GetInstance().SetPaused(false);
						GameManager::GetInstance().ForceShowCursor(FALSE);
					});
				}
				else if (key == "go_to_title")
				{
					if (optionPanel) btn->SetOnClick([this]()
						{
							optionPanel->SetActive(false);
							GameManager::GetInstance().SetPaused(false);
							GameManager::GetInstance().ForceShowCursor(TRUE);
							SceneManager::GetInstance().ChangeScene("TitleScene");
							
						});
				}


		}
		else if (auto* slider = dynamic_cast<Slider*>(uiPtr.get())) {
			std::string key = slider->GetActionKey();
			if (key == "Master_Volume")
			{
				slider->AddListener([](float val) {
					SoundManager::GetInstance().SetVolume_Main(val);
					});
			}
			else if (key == "BGM_Volume") {
				slider->AddListener([](float val) {
					SoundManager::GetInstance().SetVolume_BGM(val);
					});
			}
			else if (key == "SFX_Volume") {
				slider->AddListener([](float val) {
					SoundManager::GetInstance().SetVolume_SFX(val);
					});
			}
			else if (key == "Set_Sensitivity")
			{
				slider->AddListener([](float val) {
					Player::SetCameraSensitivity(val);
					});
			}
		}
		else if (auto* panel = dynamic_cast<Panel*>(uiPtr.get()))
		{
			if (panel->GetName() == "BlackPannel")
				m_tutorialDark = panel;
			else if (panel->GetName() == "TutorialPopUp")
				m_tutorialPopup = panel;
		}
	}

	if (m_tutorialDark && m_tutorialPopup)
	{
		GameManager::GetInstance().RegisterTutorialUI(m_tutorialDark, m_tutorialPopup);
	}
}

void TestScene::SetScoreUI(int score)
{
	if (score < 0) score = 0;
	if (score > 999999) score = 999999;

	int d1 = score % 10;
	int d10 = (score / 10) % 10;
	int d100 = (score / 100) % 10;
	int d1000 = (score / 1000) % 10;
	int d10000 = (score / 10000) % 10;
	int d100000 = (score / 100000) % 10;

	if (n1)      n1->SetTextureAndOffset("UI_n" + std::to_string(d1) + ".png");
	if (n10)     n10->SetTextureAndOffset("UI_n" + std::to_string(d10) + ".png");
	if (n100)    n100->SetTextureAndOffset("UI_n" + std::to_string(d100) + ".png");
	if (n1000)   n1000->SetTextureAndOffset("UI_n" + std::to_string(d1000) + ".png");
	if (n10000)  n10000->SetTextureAndOffset("UI_n" + std::to_string(d10000) + ".png");
	if (n100000) n100000->SetTextureAndOffset("UI_n" + std::to_string(d100000) + ".png");

	if (combo)
	{
		const int multiplier = GameManager::GetInstance().GetMultiplier();
		switch (multiplier)
		{
		case 2:
			combo->SetTextureAndOffset("UI_Combo2.png");
			break;
		case 4:
			combo->SetTextureAndOffset("UI_Combo4.png");
			break;
		case 8:
			combo->SetTextureAndOffset("UI_Combo8.png");
			break;
		default:
			combo->SetTextureAndOffset("UI_Combo1.png");
			break;
		}
	}
}

void TestScene::UpdateBulletUI(int bulletCount)
{
	if (bullet)  bullet->SetTextureAndOffset("UI_Bullet0" + std::to_string(bulletCount) + ".png");
}


void TestScene::UpdateHPUI(float hpRatio)
{
	if (!hpBar)
		return;

	hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);

	const auto offset = hpBar->GetTextureOffset();
	const float texWidth = offset.x * 2.0f;
	const float texHeight = offset.y * 2.0f;
	if (texWidth <= 0.0f || texHeight <= 0.0f)
		return;

	RECT src = {};
	src.left = 0;
	src.top = 0;
	src.right = static_cast<LONG>(texWidth * hpRatio);
	src.bottom = static_cast<LONG>(texHeight);

	hpBar->SetSourceRect(src);

	if (hpDeco)
		hpDeco->ClearSourceRect();
}

void TestScene::UpdateDeadEyeUI(float ratio)
{
	if (!deadEye)
		return;

	ratio = std::clamp(ratio, 0.0f, 1.0f);

	const auto offset = deadEye->GetTextureOffset();
	const float texWidth = offset.x * 2.0f;
	const float texHeight = offset.y * 2.0f;
	if (texWidth <= 0.0f || texHeight <= 0.0f)
		return;

	RECT src = {};
	src.left = 0;
	src.top = 0;
	src.right = static_cast<LONG>(texWidth * ratio);
	src.bottom = static_cast<LONG>(texHeight);

	deadEye->SetSourceRect(src);
}
