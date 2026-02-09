#pragma once
#include "UIBase.h"

class Panel : public UIBase
{
	friend class SceneBase;

public:
	Panel();
	void OnResize(std::pair<float, float> res) override;
	void RenderUI(class Renderer& renderer) override;
	void AddChild(std::unique_ptr<UIBase> child) { child->SetParent(this); m_children.emplace_back(std::move(child)); }

	std::string GetTypeName() const override { return "Panel"; }


protected:
	void UpdateRect() override;

	std::vector<std::unique_ptr<UIBase>> m_children;
};
