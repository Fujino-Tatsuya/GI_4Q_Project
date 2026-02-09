#pragma once
#include "SceneBase.h"
#include <DirectXMath.h>


class TitleScene : public SceneBase
{
	class Panel* optionPanel = nullptr;
	class Panel* creditPanel = nullptr;
	class Panel* Titles = nullptr;
	class Panel* Title_letterrbox_down = nullptr;
	class Panel* Title_letterrbox_up = nullptr;

	float m_time4MovingPanel = 0;
	bool m_letterboxInit = false;
	DirectX::XMFLOAT2 m_letterboxUpStartPos = {};
	DirectX::XMFLOAT2 m_letterboxDownStartPos = {};
	float m_letterboxUpStartDepth = 0.0f;
	float m_letterboxDownStartDepth = 0.0f;

	const float kTime4MovingPanel = 1.0f;
	const DirectX::XMFLOAT2 upTargetPos = { 0.5f, -0.04f };
	const DirectX::XMFLOAT2 downTargetPos = { 0.5f, 1.39f };
	const float targetDepth = 0.0f;

private:
	void Initialize() override;
	void Update() override;
	void BindUIActions() override;
	
	void MovingPanel(float dt);

public:
	TitleScene() = default;
	~TitleScene() override = default;
	TitleScene(const TitleScene&) = default;
	TitleScene& operator=(const TitleScene&) = default;
	TitleScene(TitleScene&&) = default;
	TitleScene& operator=(TitleScene&&) = default;
};
