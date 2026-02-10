#include "stdafx.h"

#include "ResourceManager.h"
#include "Renderer.h"

#include "UIBase.h"
#include "AnimPanel.h"
#include "Button.h"
#include "Panel.h"
#include "Slider.h"
#include "Text.h"

using namespace nlohmann;

DirectX::XMFLOAT2 UIBase::GetWorldPosition() const
{
	auto& rm = Renderer::GetInstance();

	DirectX::XMFLOAT2 pixelPos =
	{
		m_localPosition.x * rm.GetCurResolution().first,
		m_localPosition.y * rm.GetCurResolution().second
	};

	if (m_parent)
	{
		auto parentPos = m_parent->GetWorldPosition();
		return { parentPos.x + pixelPos.x,
				 parentPos.y + pixelPos.y };
	}

	return pixelPos;
}

void UIBase::SetTextureAndOffset(const std::string& idle)
{
	m_pathIdle = idle;
	m_textureIdle = ResourceManager::GetInstance().GetTextureAndOffset(idle);
	UpdateRect();
}

json UIBase::Serialize() const
{
	json data;
	data["type"] = GetTypeName();
	data["name"] = m_name;
	data["active"] = m_isActive;

	data["pos"] = { m_localPosition.x, m_localPosition.y };
	data["scale"] = GetFinalScale();

	data["depth"] = m_depth;
	data["pathIdle"] = m_pathIdle;

	DirectX::XMFLOAT4 color;
	DirectX::XMStoreFloat4(&color, m_colorIdle);
	data["colorIdle"] = { color.x, color.y, color.z, color.w };

	return data;
}

void UIBase::Deserialize(const json& data)
{
	if (data.contains("name")) m_name = data["name"];
	if (data.contains("active")) m_isActive = data["active"];

	if (data.contains("pos")) {
		m_localPosition.x = data["pos"][0];
		m_localPosition.y = data["pos"][1];
	}
	if (data.contains("scale")) m_designScale = data["scale"];
	if (data.contains("depth")) m_depth = data["depth"];
	if (data.contains("pathIdle"))
	{
		m_pathIdle = data.value("pathIdle", "");
		if (!m_pathIdle.empty())
			SetTextureAndOffset(m_pathIdle);
	}

	if (data.contains("colorIdle"))
	{
		DirectX::XMFLOAT4 color = {};
		color.x = data["colorIdle"][0];
		color.y = data["colorIdle"][1];
		color.z = data["colorIdle"][2];
		color.w = data["colorIdle"][3];
		m_colorIdle = DirectX::XMLoadFloat4(&color);
	}

	UpdateRect();
}

UIBase* UIBase::CreateFactory(const std::string& typeName)
{
	if (typeName == "Button")      return new Button();
	if (typeName == "AnimPanel")   return new AnimPanel();
	if (typeName == "Panel")       return new Panel();
	if (typeName == "Slider")      return new Slider();
	if (typeName == "Text")        return new Text();

	return nullptr;
}
