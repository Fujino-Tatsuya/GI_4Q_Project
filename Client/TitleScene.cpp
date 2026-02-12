#include "stdafx.h"
#include "TitleScene.h"

#include "SceneManager.h"
#include "CameraComponent.h"
#include "SoundManager.h"
#include "TimeManager.h"
#include "GameManager.h"

#include "UIBase.h"
#include "Button.h"
#include "Panel.h"
#include "Slider.h"
#include "Text.h"


#include "Shared/Config/Option.h"

REGISTER_TYPE(TitleScene);

void TitleScene::Initialize()
{
	GameManager::GetInstance().ForceShowCursor(TRUE);
	GameManager::GetInstance().OnSceneEnter(EScene::Title);

	GetRootGameObject("MainCam")->GetComponent<class CameraComponent>()->SetAsMainCamera();

	for (const auto& uiPtr : m_UIList) {
		if (auto* panel = dynamic_cast<Panel*>(uiPtr.get())) {
			if (panel->GetName() == "titles") Titles = panel;
		}
	}

	if (optionPanel) optionPanel->SetActive(false);
	if (creditPanel) creditPanel->SetActive(false);
	if (Titles) Titles->SetActive(false);
	if (Title_letterrbox_down) Title_letterrbox_down->SetActive(true);
	if (Title_letterrbox_up) Title_letterrbox_up->SetActive(true);
}

void TitleScene::Update()
{
	GameManager::GetInstance().OnSceneUpdate();

	float dt = TimeManager::GetInstance().GetDeltaTime();

	m_startTimeForMove += dt;

	if (m_startTimeForMove > 0)
	{
		MovingPanel(dt);
	}
	
	if (m_PanelAnimationEnd)
	{
		Titles->UpdateFade(dt);
	}
	
}

void TitleScene::Finalize()
{
	m_startTimeForMove = -1.0f;
	m_PanelAnimationEnd = false;
}

void TitleScene::BindUIActions()
{
	Panel* sceneCheatPanel = nullptr;
	for (const auto& uiPtr : m_UIList) {
		if (auto* panel = dynamic_cast<Panel*>(uiPtr.get())) {
			if (panel->GetName() == "option") optionPanel = panel;
			else if (panel->GetName() == "UI_Title_letterrbox_down") Title_letterrbox_down = panel;
			else if (panel->GetName() == "UI_Title_letterrbox_up") Title_letterrbox_up = panel;
			else if (panel->GetName() == "credit") creditPanel = panel;
			else if (panel->GetName() == "Titles") Titles = panel;
		} else if (auto* text = dynamic_cast<Text*>(uiPtr.get())) {
			//if (text->GetName() == "result_time") resultTime = text;
		}
	}
	GameManager::GetInstance().RegisterCheatPanel(sceneCheatPanel);


	for (auto& uiPtr : m_UIList) {
		// -------------------------------------------------------
		// 1. Button bindings
		// -------------------------------------------------------
		if (auto* btn = dynamic_cast<Button*>(uiPtr.get())) {
			std::string key = btn->GetActionKey();

			 if (key == "start_game") {
				btn->SetOnClick([this]() { if (m_isPanel) return; SceneManager::GetInstance().ChangeScene("TestScene");  });
			} else if (key == "quit_game") {
				btn->SetOnClick([this]() {
					if (m_isPanel) return;
				PostQuitMessage(0); });
			} else if (key == "open_option") {
				if (optionPanel) btn->SetOnClick([this]() {
					if (m_isPanel) return;
					optionPanel->SetActive(true);
					m_isPanel = true;
					});
			} else if (key == "close_option") {
				if (optionPanel) btn->SetOnClick([this]() {
					optionPanel->SetActive(false);
					m_isPanel = false;
					});
			} else if (key == "open_credit") {
				if (optionPanel) btn->SetOnClick([this]() { 
					if (m_isPanel) return;
					creditPanel->SetActive(true); 
					m_isPanel = true;
					});
			} else if (key == "close_credit") {
				if (optionPanel) btn->SetOnClick([this]() { 
					creditPanel->SetActive(false);
					m_isPanel = false;
					});
			}
		}

		// -------------------------------------------------------
		// 2. Slider bindings
		// -------------------------------------------------------
		else if (auto* slider = dynamic_cast<Slider*>(uiPtr.get())) {
			std::string key = slider->GetActionKey();

			if (key == "Master_Volume") {
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
	}
}


void TitleScene::MovingPanel(float dt)
{
	if (!Title_letterrbox_down || !Title_letterrbox_up) return;

	if (m_PanelAnimationEnd) return;

	if (m_time4MovingPanel >= kTime4MovingPanel) return;
	
	if (!m_letterboxInit)
	{
		m_letterboxUpStartPos = Title_letterrbox_up->GetLocalPosition();
		m_letterboxDownStartPos = Title_letterrbox_down->GetLocalPosition();
		m_letterboxUpStartDepth = Title_letterrbox_up->GetDepth();
		m_letterboxDownStartDepth = Title_letterrbox_down->GetDepth();
		m_letterboxInit = true;
	}


	m_time4MovingPanel += dt;
	if (m_time4MovingPanel > kTime4MovingPanel) m_time4MovingPanel = kTime4MovingPanel;

	const float t = m_time4MovingPanel / kTime4MovingPanel;
	auto Lerp = [](float a, float b, float s) { return a + (b - a) * s; };

	Title_letterrbox_up->SetLocalPosition(
		{ Lerp(m_letterboxUpStartPos.x, upTargetPos.x, t), Lerp(m_letterboxUpStartPos.y, upTargetPos.y, t) });
	Title_letterrbox_down->SetLocalPosition(
		{ Lerp(m_letterboxDownStartPos.x, downTargetPos.x, t), Lerp(m_letterboxDownStartPos.y, downTargetPos.y, t) });

	Title_letterrbox_up->SetDepth(Lerp(m_letterboxUpStartDepth, targetDepth, t));
	Title_letterrbox_down->SetDepth(Lerp(m_letterboxDownStartDepth, targetDepth, t));

	if (t > 0.99f && !m_PanelAnimationEnd)
	{
		if (Titles)
		{
			Titles->StartFadeIn(1.5f);
			Titles->SetActive(true);
		}

		m_PanelAnimationEnd = true;
	}
}
