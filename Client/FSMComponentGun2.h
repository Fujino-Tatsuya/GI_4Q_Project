///BOF FSMComponentGun2.h
#pragma once
#include "FSMComponent.h"
#include <DirectXMath.h>

class FSMComponentGun2 : public FSMComponent
{
	float m_timer = 0.0f;

	DirectX::XMVECTOR m_originRotGun{ 0,0,0,0 };     // 총 전체
	DirectX::XMVECTOR m_originRotCylinder{ 0,0,0,0 }; // 실린더
	DirectX::XMVECTOR m_originRotPin{ 0,0,0,0 };      // 공이

	float m_targetCylinderAngleX = 0.0f;

	class GameObjectBase* gun		= nullptr; // 주인
	class GameObjectBase* body		= nullptr;
	class GameObjectBase* cylinder	= nullptr;
	class GameObjectBase* pin		= nullptr;

protected:
	enum EState
	{
		EIdle,   // 0
		EAttack, 
		EReload, 
		ECount
	};

public:
	FSMComponentGun2() = default;
	~FSMComponentGun2() override = default;

	std::string StateToString(StateID state) const override;
	StateID StringToState(const std::string& str) const override;
	
	void Fire();
	void Reload();

protected:
	void Initialize() override;

	void OnEnterState(StateID state) override;
	void OnUpdateState(StateID state) override;
	void OnExitState(StateID state) override;

	#ifdef _DEBUG
	void RenderImGui() override;
	#endif



};
///EOF FSMComponentGun2.h