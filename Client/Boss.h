#pragma once
#include "GameObjectBase.h"

class Boss : public GameObjectBase
{
	class Player* m_player = nullptr;

	float m_moveSpeed = 4.0f;

	class FSMComponentBoss* m_fsm = nullptr;

	float m_deathTimer = 0.0f;
	const float m_deathDuration = 2.0f;

	bool m_hasFoundPlayer = true;
	const float m_attackRangeSquare = 5.0f;

	std::string m_triggerColliderName;
	class ColliderComponent* m_triggerCollider = nullptr;

public:
	enum class AIState
	{
		Idle,
		Chase,
		Attack,
		Dead
	};
	AIState m_state = AIState::Chase;

	Boss() = default;
	~Boss() override = default;
	Boss(const Boss&) = default;
	Boss& operator=(const Boss&) = default;
	Boss(Boss&&) = default;
	Boss& operator=(Boss&&) = default;

	void Die();
	void OnAttackFinished();

private:
	void Initialize() override;
	void Update() override;
	#ifdef _DEBUG
	void RenderImGui() override;
	#endif

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;
};

