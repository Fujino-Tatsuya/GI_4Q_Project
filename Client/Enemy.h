#pragma once
#include "GameObjectBase.h"

class Enemy : public GameObjectBase
{
	static std::vector<Enemy*> s_enemies;

	bool m_isTutorialDummy = false;

	class Player* m_player = nullptr;

	float m_moveSpeedSquared = 4.0f;
	std::deque<DirectX::XMVECTOR> m_path = {};
	const float m_pathFindInterval = 1.0f;
	float m_pathFindTimer = 0.0f;
	float m_pathFindIntervalRandomOffset = 0.0f;

	class FSMComponentEnemy* m_fsm = nullptr;

	float m_deathTimer = 0.0f;
	const float m_deathDuration = 2.0f;

	bool m_hasFoundPlayer = false;
	const float m_attackRangeSquare = 5.0f;

	std::string m_triggerColliderName;
	class ColliderComponent* m_triggerCollider = nullptr;

	float m_rotationSpeed = 10.0f;
	const float m_separationRadius = 0.8f;
	const float m_separationStrength = 3.5f;
	const float m_separationMaxPush = 2.0f;

public:
	enum class AIState
	{
		Idle,
		Chase,
		Attack,
		Dead
	};
	AIState m_state = AIState::Chase;
	static const std::vector<Enemy*>& GetEnemies() { return s_enemies; }

	Enemy() = default;
	~Enemy() override = default;
	Enemy(const Enemy&) = default;
	Enemy& operator=(const Enemy&) = default;
	Enemy(Enemy&&) = default;
	Enemy& operator=(Enemy&&) = default;

	void Die();
	void OnAttackFinished();
	class Player* GetTargetPlayer() const { return m_player; }

	void SetAsTutorialDummy();

private:
	void Initialize() override;
	void Update() override;
	#ifdef _DEBUG
	void RenderImGui() override;
	#endif
	void Finalize() override;

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;

	void MoveAlongPath(float dt);
	void ApplySeparation(float dt);
};
