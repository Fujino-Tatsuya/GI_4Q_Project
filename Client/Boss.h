#pragma once
#include "GameObjectBase.h"

class Boss : public GameObjectBase
{
	class Player* m_player = nullptr;

	int m_hitPoints = 30;
	int m_maxHitPoints = 30;

	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_bossHealthBarTextureAndOffset = {};
	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_bossHealthBarDecoTextureAndOffset = {};
	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_bossHealthBarBackgroundTextureAndOffset = {};

	float m_moveSpeed = 5.0f;

	class FSMComponentBoss* m_fsm = nullptr;

	float m_deathTimer = 0.0f;
	const float m_deathDuration = 2.0f;

	bool m_hasFoundPlayer = false;
	const float m_attackRangeSquare = 8.0f;

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

	void Hit() { m_hitPoints--; if (m_hitPoints <= 0) Die(); }
	void Die();
	void OnAttackFinished();

private:
	void Initialize() override;
	void Update() override;
	void Render() override;
	#ifdef _DEBUG
	void RenderImGui() override;
	#endif

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;
};

