#include "stdafx.h"
#include "Boss.h"

#include "ColliderComponent.h"
#include "FSMComponentBoss.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Player.h"
#include "TimeManager.h"

REGISTER_TYPE(Boss)

using namespace std;
using namespace DirectX;

void Boss::Die()
{
	if (m_state == AIState::Dead) return;

	m_state = AIState::Dead;
	m_deathTimer = 0.0f;
	if (m_fsm) m_fsm->ChangeState(FSMComponentBoss::EDead);
}

void Boss::OnAttackFinished()
{
	if (m_state == AIState::Dead) return;

	m_state = AIState::Chase;
	if (m_fsm) m_fsm->ChangeState(FSMComponentBoss::EChase);
}

void Boss::Initialize()
{
	m_fsm = GetComponent<FSMComponentBoss>();

	m_player = static_cast<Player*>(SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive("Player"));
	if (!m_player) cout << "Boss 초기화 오류: Player 게임 오브젝트를 찾을 수 없습니다." << endl;

	GameObjectBase* triggerObj = SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive(m_triggerColliderName);
	if (triggerObj) m_triggerCollider = triggerObj->GetComponent<ColliderComponent>();

	if (m_state == AIState::Chase && m_fsm) { m_fsm->ChangeState(FSMComponentBoss::EChase); }
}

void Boss::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	if (!m_player) return;
	const XMVECTOR& playerPos = m_player->GetPosition();

	if (m_triggerCollider && !m_hasFoundPlayer && m_triggerCollider->CheckCollisionPoint(playerPos)) m_hasFoundPlayer = true;

	switch (m_state)
	{
	case AIState::Idle:
		break;

	case AIState::Chase:
	{
		if (!m_hasFoundPlayer) break;

		XMVECTOR toPlayer = XMVectorSubtract(playerPos, GetPosition());
		float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));

		if (distSq <= m_attackRangeSquare)
		{
			m_state = AIState::Attack;
			if (m_fsm) m_fsm->ChangeState(FSMComponentBoss::EAttack);
		}
		else
		{
			LookAt(XMVectorSetY(playerPos, 0.0f));
			MoveDirection(m_moveSpeed * deltaTime, Direction::Forward);
			Rotate({ 0.0f, 180.0f , 0.0f });
		}
		break;
	}

	case AIState::Attack:
		LookAt(XMVectorSetY(playerPos, 0.0f));
		Rotate({ 0.0f, 180.0f , 0.0f });
		break;

	case AIState::Dead:
		m_deathTimer += deltaTime;
		if (m_deathTimer >= m_deathDuration) SetAlive(false);
		return;

	default:
		break;
	}
}

#ifdef _DEBUG
void Boss::RenderImGui()
{
	array<char, 256> triggerColliderNameBuffer = {};
	strcpy_s(triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size(), m_triggerColliderName.c_str());
	if (ImGui::InputText("Trigger Collider Name", triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size())) m_triggerColliderName = string(triggerColliderNameBuffer.data());
}
#endif

nlohmann::json Boss::Serialize()
{
	nlohmann::json jsonData = {};
	jsonData["triggerColliderName"] = m_triggerColliderName;
	return jsonData;
}

void Boss::Deserialize(const nlohmann::json& jsonData)
{
	if (jsonData.find("triggerColliderName") != jsonData.end()) m_triggerColliderName = jsonData["triggerColliderName"].get<string>();
}