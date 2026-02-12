#include "stdafx.h"
#include "FSMComponentBoss.h"

#include "GameObjectBase.h"
#include "SkinnedModelComponent.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Boss.h"
#include "Player.h"
#include "ParticleObject.h"
#include "../Engine/Animator.h"
#include "GameManager.h"

REGISTER_TYPE(FSMComponentBoss)

using namespace std;
using namespace DirectX;

namespace
{
	DissolveBuffer kHP30 = { 0, 0, 0, 0, { 1.0f, 0.5f, 0.0f, 1.0f } };
	DissolveBuffer kHP20 = { 0.23f, 0.27f, 6, 0, { 1.0f, 0.5f, 0.0f, 1.0f } };
	DissolveBuffer kHP10 = { 0, 1, 6.7f, 0, { 1.0f, 0.5f, 0.0f, 1.0f } };
	DissolveBuffer kHP0  = { 0.32f,  0.33f, 8.2f, 0, { 1.0f, 0.5f, 0.0f, 1.0f } };
}

void FSMComponentBoss::Initialize()
{
	m_model = GetOwner()->GetComponent<SkinnedModelComponent>();

	m_owner_boss_ = dynamic_cast<Boss*>(GetOwner());
	if (auto* scene = SceneManager::GetInstance().GetCurrentScene()) m_player = dynamic_cast<Player*>(scene->GetRootGameObject("Player"));
	FSMComponent::Initialize();
}

std::string FSMComponentBoss::StateToString(StateID state) const
{
	switch (state)
	{
	case EChase: return "Chase";
	case EAttack: return "Attack";
	case EJump: return "Jump";
	case EDead: return "Dead";
	default: return "Unknown";
	}
}

FSMComponent::StateID FSMComponentBoss::StringToState(const std::string& str) const
{
	if (str == "Chase") return EChase;
	if (str == "Attack") return EAttack;
	if (str == "Jump") return EJump;
	if (str == "Dead") return EDead;
	return EChase;
}

void FSMComponentBoss::OnEnterState(StateID state)
{
	if (!m_model || !m_model->GetAnimator()) return;

	switch (state)
	{
	case EChase:
		// 2 = run
		m_model->GetAnimator()->SetPlaybackSpeed(1.0f);
		m_model->GetAnimator()->PlayAnimation(2, true);
		m_model->SetBlendState(BlendState::Opaque);
		break;

	case EAttack:
		// 0 = attack
		m_attack_timer = 0.0f;
		m_attack_has_hit = false;
		m_model->GetAnimator()->SetPlaybackSpeed(1.0f);
		m_model->GetAnimator()->PlayAnimation(0, false);
		break;

	case EJump:
		// 1 = jump attack
		m_jump_timer = 0.0f;
		m_jump_has_hit = false;
		m_model->GetAnimator()->SetPlaybackSpeed(1.0f);
		m_model->GetAnimator()->PlayAnimation(1, false);
		break;

	case EDead:
		m_death_timer = 0.0f;
		m_model->GetAnimator()->SetPlaybackSpeed(0.1f);
		m_model->GetAnimator()->PlayAnimation(0, false);
		m_model->SetBlendState(BlendState::AlphaBlend);
		break;
	}
}

void FSMComponentBoss::OnUpdateState(StateID state)
{
	const float dt = TimeManager::GetInstance().GetDeltaTime();
	if (m_model && m_owner_boss_ && state != EDead)
	{
		const int hp = m_owner_boss_->GetHitPoints();
		if (hp <= 0)
		{
			m_model->SetDissolveData(kHP0);
		}
		else if (hp <= 10)
		{
			m_model->SetDissolveData(kHP10);
		}
		else if (hp <= 20)
		{
			m_model->SetDissolveData(kHP20);
		}
		else
		{
			m_model->SetDissolveData(kHP30);
		}
	}

	switch (state)
	{
	case EAttack:
		m_attack_timer += dt;
		if (m_attack_timer >= kAttackAnticipation && !m_attack_has_hit)
		{
			m_attack_has_hit = true;

			if (!m_player)
			{
				if (auto* scene = SceneManager::GetInstance().GetCurrentScene())
				{
					m_player = dynamic_cast<Player*>(scene->GetRootGameObject("Player"));
				}
			}

			if (m_player && m_owner_boss_)
			{
				const XMVECTOR diff = m_player->GetPosition() - m_owner_boss_->GetPosition();
				const float distSq = XMVectorGetX(XMVector3LengthSq(diff));
				if (distSq <= kAttackRange * kAttackRange)
				{
					for (int i = 0; i < kDamage; ++i)
					{
						m_player->TakeHit();
					}
				}
			}
		}
		if (m_attack_timer >= kAttackTotalTime && m_owner_boss_) m_owner_boss_->OnAttackFinished();
		break;

	case EJump:
		m_jump_timer += dt;
		if (m_jump_timer >= kJumpAttackAnticipation - 0.5f && !m_jump_has_hit)
		{
			m_jump_has_hit = true;

			ParticleObject* bossDust = dynamic_cast<ParticleObject*>(m_owner_boss_->CreatePrefabChildGameObject("BossDust.json"));
			bossDust->SetPosition(m_owner_boss_->GetPosition());
			bossDust->SetLifetime(1.0f);

			if (!m_player)
			{
				if (auto* scene = SceneManager::GetInstance().GetCurrentScene())
				{
					m_player = dynamic_cast<Player*>(scene->GetRootGameObject("Player"));
				}
			}

			if (m_player && m_owner_boss_)
			{
				const XMVECTOR diff = m_player->GetPosition() - m_owner_boss_->GetPosition();
				const float distSq = XMVectorGetX(XMVector3LengthSq(diff));
				if (distSq <= kAttackRange * kAttackRange)
				{
					for (int i = 0; i < kDamage; ++i)
					{
						m_player->TakeHit();
					}
				}
			}
		}
		if (m_jump_timer >= kJumpTotalTime && m_owner_boss_) m_owner_boss_->OnAttackFinished();
		break;

	case EDead:
		m_death_timer += dt;
		if (m_death_timer >= kFadeStartTime)
		{
			float progress = (m_death_timer - kFadeStartTime) / kFadeDuration;
			if (progress > 1.0f)
			{
				GameManager::GetInstance().SetSuccess(true);
				SceneManager::GetInstance().ChangeScene("EndingScene");
			};

			if (m_model)
			{
				m_model->SetAlpha(1.0f - progress);
				m_model->SetDissolveThreshold(progress);
			}
		}
		break;
	}
}

void FSMComponentBoss::OnExitState(StateID state)
{
	(void)state;
}

#ifdef _DEBUG
void FSMComponentBoss::RenderImGui()
{
	if (ImGui::TreeNode("FSM Component Boss"))
	{
		const string currentName = StateToString(current_state_);
		ImGui::Text("Current State: %s", currentName.c_str());

		if (ImGui::BeginCombo("Force State", currentName.c_str()))
		{
			for (int i = 0; i < ECount; ++i)
			{
				const EState state = static_cast<EState>(i);
				const string stateName = StateToString(state);
				const bool isSelected = (current_state_ == state);
				if (ImGui::Selectable(stateName.c_str(), isSelected))
				{
					ChangeState(state);
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::TreePop();
	}
}
#endif
