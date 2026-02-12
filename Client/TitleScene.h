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

	struct MenuItem
	{
		std::string key;
		class Text* text = nullptr;
		class Button* button = nullptr;
		class Panel* panel = nullptr;
	};

	std::array<MenuItem, 4> m_menuItems =
	{
		MenuItem{ "start_game", nullptr, nullptr , nullptr },
		MenuItem{ "open_option", nullptr, nullptr, nullptr  },
		MenuItem{ "open_credit", nullptr, nullptr, nullptr  },
		MenuItem{ "quit_game", nullptr, nullptr, nullptr  }
	};

	bool m_isPanel = false;

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

	float m_startTimeForMove = -2.0f;
	bool m_PanelAnimationEnd = false;

private:
	void Initialize() override;
	void Update() override;
	void Render() override;
	void Finalize() override;

	void BindUIActions() override;
	
	void MovingPanel(float dt);
	void UpdateMenuTextColors();

public:
	TitleScene() = default;
	~TitleScene() override = default;
	TitleScene(const TitleScene&) = default;
	TitleScene& operator=(const TitleScene&) = default;
	TitleScene(TitleScene&&) = default;
	TitleScene& operator=(TitleScene&&) = default;
};
