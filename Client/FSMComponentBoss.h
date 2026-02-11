#pragma once
#include "FSMComponent.h"

class FSMComponentBoss : public FSMComponent
{
public:
	enum EState
	{
		EChase,
		EAttack,
		EJump,
		EDead,
		ECount
	};

public:
	FSMComponentBoss() = default;
	~FSMComponentBoss() override = default;

	std::string StateToString(StateID state) const override;
	StateID StringToState(const std::string& str) const override;

	void SetModelComponent(class SkinnedModelComponent* model) { m_model = model; }

protected:
	void OnEnterState(StateID state) override;
	void OnUpdateState(StateID state) override;
	void OnExitState(StateID state) override;
	void Initialize() override;
#ifdef _DEBUG
	void RenderImGui() override;
#endif

private:
	class SkinnedModelComponent* m_model = nullptr;
	class Player* m_player = nullptr;

	float m_death_timer = 0.0f;
	float m_attack_timer = 0.0f;
	float m_jump_timer = 0.0f;
	bool m_attack_has_hit = false;
	bool m_jump_has_hit = false;

	const float kAttackRange = 3.2f;
	const float kAttackAnticipation = 40.0f / 24.0f;
	const float kAttackTotalTime = 59.0f / 24.0f;
	const float kJumpAttackAnticipation = 60.0f / 24.0f;
	const float kJumpTotalTime = 76.0f / 24.0f;
	const int kDamage = 1;

	const float kFadeStartTime = 0.5f;
	const float kFadeDuration = 1.5f;
};
