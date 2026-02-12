#include "stdafx.h"
#include "UIBase.h"
#include "Slider.h"

#include "ResourceManager.h"
#include "Renderer.h"

Slider::Slider()
{
	SetTextureAndOffset("UI_Slider_box.png"); 
	SetHandleTextures("UI_Slider.png", "UI_Slider.png", "UI_Slider.png");
}

void Slider::SetRange(float min, float max)
{
	if (min > max) std::swap(min, max);
	m_min = min;
	m_max = max;
	m_rendervalue = std::clamp(m_rendervalue, m_min, m_max);
	UpdateHandleRect();
}

void Slider::SetValue(float newValue)
{
	float safeMin = std::min(m_min, m_max);
	float safeMax = std::max(m_min, m_max);
	float clampedValue = std::clamp(newValue, safeMin, safeMax);

	float prevValue = m_realvalue;

	if (fabs(prevValue - clampedValue) < 0.0001f)
	{
		m_rendervalue = clampedValue;
		return;
	}

	m_realvalue = clampedValue;
	m_rendervalue = clampedValue;

	UpdateHandleRect();
	NotifyValueChanged();
}

void Slider::SetHandleTexture(const std::string& tex)
{
	SetHandleTextures(tex, tex, tex);
}

void Slider::SetHandleTextureJSON(const std::string& path)
{
	if (!path.empty())
		SetHandleTextures(path, path, path);
}

void Slider::SetHandleTextures(const std::string& idle, const std::string& hover, const std::string& pressed)
{
	m_handlePathIdle = idle;
	m_handlePathHover = hover.empty() ? idle : hover;
	m_handlePathPressed = pressed.empty() ? idle : pressed;

	auto& rm = ResourceManager::GetInstance();
	if (!m_handlePathIdle.empty()) m_handleTexIdle = rm.GetTextureAndOffset(m_handlePathIdle);
	if (!m_handlePathHover.empty()) m_handleTexHover = rm.GetTextureAndOffset(m_handlePathHover);
	if (!m_handlePathPressed.empty()) m_handleTexPressed = rm.GetTextureAndOffset(m_handlePathPressed);

	UpdateHandleRect();
}

void Slider::AddListener(std::function<void(float)> callback)
{
	listeners.push_back(callback);
}

void Slider::NotifyValueChanged()
{
	for (auto& cb : listeners)
		cb(m_rendervalue);
}

void Slider::RenderUI(Renderer& renderer)
{
	if (!IsActuallyActive())
		return;

	// --- [Background] ---
	auto barTex = m_textureIdle.first;
	auto barPos = GetWorldPosition();
	auto barOff = m_textureIdle.second;
	auto scale = GetFinalScale();
	auto color = GetFinalColor(m_colorIdle);
	auto depth = m_depth;

	renderer.UI_RENDER_FUNCTIONS().emplace_back(
		[barTex, barPos, barOff, scale, color, depth]()
		{
			Renderer::GetInstance().RenderImageUIPosition(
				barTex, barPos, barOff, scale, color, depth
			);
		});

	UpdateHandleRect();

	DirectX::XMFLOAT2 handlePos = m_cachedHandleCenter;

	float range = m_max - m_min;
	float t = (range != 0.0f) ? (m_rendervalue - m_min) / range : 0.0f;
	t = std::clamp(t, 0.0f, 1.0f);

	auto pos = GetWorldPosition();

	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> currentTexPair;
	float currentHandleScaleMult = 1.0f;
	DirectX::XMVECTOR currentHandleColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	switch (m_handleState)
	{
	case HandleState::Pressed:
		currentTexPair = m_handleTexPressed;
		currentHandleScaleMult = m_handleScalePressed;
		currentHandleColor = m_handleColorPressed;
		break;
	case HandleState::Hover:
		currentTexPair = m_handleTexHover;
		currentHandleScaleMult = m_handleScaleHover;
		currentHandleColor = m_handleColorHover;
		break;
	case HandleState::Idle:
	default:
		currentTexPair = m_handleTexIdle;
		currentHandleScaleMult = m_handleScaleIdle;
		currentHandleColor = m_handleColorIdle;
		break;
	}

	if (!currentTexPair.first) currentTexPair = m_handleTexIdle;

	auto handleTex = currentTexPair.first;
	auto handleOff = currentTexPair.second;


	float finalScale = GetFinalScale() * currentHandleScaleMult;

	renderer.UI_RENDER_FUNCTIONS().emplace_back(
		[handleTex, handlePos, handleOff, finalScale, currentHandleColor, depth]()
		{
			Renderer::GetInstance().RenderImageScreenPosition(
				handleTex,
				handlePos,
				handleOff,
				finalScale,
				currentHandleColor,
				depth + 0.01f
			);
		});

	if (m_showValueText)
	{
		int percent = (int)std::round((m_rendervalue / m_max) * 100.0f);
		std::wstring valueText = std::to_wstring(percent);

		DirectX::XMFLOAT2 textPos =
		{
			handlePos.x + m_valueTextOffset.x,
			handlePos.y + m_valueTextOffset.y
		};

		auto textColor = m_valueTextColor;
		float textScale = m_valueTextScale;
		
		auto* font = ResourceManager::GetInstance().GetSpriteFont(L"Paperlogy");

		DirectX::XMFLOAT2 finalTextPos = textPos;

		if (font)
		{
			DirectX::XMVECTOR sizeVec = font->MeasureString(valueText.c_str());
			DirectX::XMFLOAT2 size;
			DirectX::XMStoreFloat2(&size, sizeVec);

			float scaledWidth = size.x * textScale;
			float scaledHeight = size.y * textScale;

			finalTextPos.x -= scaledWidth * 0.5f;
		}

		renderer.UI_RENDER_FUNCTIONS().emplace_back(
			[valueText, finalTextPos, depth, textColor, textScale]()
			{
				Renderer::GetInstance().RenderTextScreenPosition(
					valueText.c_str(),
					finalTextPos,
					depth + 0.1f,
					textColor,
					textScale,
					L"Paperlogy"
				);
			});
	}
}

void Slider::UpdateRect()
{
	auto pos = GetWorldPosition();

	float halfW = m_textureIdle.second.x * 0.5f * GetFinalScale();
	float halfH = m_textureIdle.second.y * 0.5f * GetFinalScale();

	m_UIRect =
	{
		static_cast<LONG>(pos.x - halfW),
		static_cast<LONG>(pos.y - halfH),
		static_cast<LONG>(pos.x + halfW),
		static_cast<LONG>(pos.y + halfH)
	};

	UpdateHandleRect();
}

void Slider::UpdateHandleRect()
{
	float denom = (m_max - m_min);
	float t = (fabs(denom) > 0.0001f) ? (m_rendervalue - m_min) / denom : 0.0f;
	t = std::clamp(t, 0.0f, 1.0f);

	float barLeft = m_UIRect.left - m_textureIdle.second.x * 0.5f * GetFinalScale();
	float barRight = m_UIRect.right + m_textureIdle.second.x * 0.5f * GetFinalScale();

	float scale = GetFinalScale() * m_handleScaleIdle;
	float halfW = m_handleTexIdle.second.x * 0.5f * scale;
	float halfH = m_handleTexIdle.second.y * 0.5f * scale;

	float minX = barLeft + halfW;
	float maxX = barRight - halfW;

	float handleX = minX + t * (maxX - minX);
	float handleY = ((float)m_UIRect.top + (float)m_UIRect.bottom) * 0.5f;

	m_handleRect =
	{
		(LONG)(handleX - halfW),
		(LONG)(handleY - halfH),
		(LONG)(handleX + halfW),
		(LONG)(handleY + halfH)
	};

	m_cachedHandleCenter = { handleX, handleY };
}

bool Slider::CheckInput(const POINT& mousePos, bool isMousePressed)
{
	if (!IsActuallyActive())
	{
		m_dragging = false;
		m_handleState = HandleState::Idle;
		return false;
	}

	UpdateHandleRect();
	const bool isOverHandle = PtInRect(&m_handleRect, mousePos);

	if (!isMousePressed)
	{
		if (m_dragging)
		{
			m_dragging = false;

			SetValue(m_rendervalue);
		}

		m_handleState = isOverHandle ? HandleState::Hover : HandleState::Idle;
		return false;
	}

	if (!m_dragging)
	{
		if (!PtInRect(&m_UIRect, mousePos) && !isOverHandle)
			return false;

		m_dragging = true;
	}

	m_handleState = HandleState::Pressed;

	float scale = GetFinalScale() * m_handleScaleIdle;
	float halfW = m_handleTexIdle.second.x * 0.5f * scale;

	float barLeft = m_UIRect.left - m_textureIdle.second.x * 0.5f * GetFinalScale();
	float barRight = m_UIRect.right + m_textureIdle.second.x * 0.5f * GetFinalScale();

	float minX = barLeft + halfW;
	float maxX = barRight - halfW;

	float t = (mousePos.x - minX) / (maxX - minX);
	t = std::clamp(t, 0.0f, 1.0f);

	m_rendervalue = m_min + t * (m_max - m_min);

	UpdateHandleRect();
	return true;
}


nlohmann::json Slider::Serialize() const
{
	nlohmann::json data = UIBase::Serialize();

	data["min"] = m_min;
	data["max"] = m_max;
	data["value"] = m_realvalue;
	data["handlePathIdle"] = m_handlePathIdle;
	data["handlePathHover"] = m_handlePathHover;
	data["handlePathPressed"] = m_handlePathPressed;

	data["handleScaleIdle"] = m_handleScaleIdle;
	data["handleScaleHover"] = m_handleScaleHover;
	data["handleScalePressed"] = m_handleScalePressed;

	DirectX::XMFLOAT4 c_i, c_h, c_p;
	DirectX::XMStoreFloat4(&c_i, m_handleColorIdle);
	DirectX::XMStoreFloat4(&c_h, m_handleColorHover);
	DirectX::XMStoreFloat4(&c_p, m_handleColorPressed);
	data["handleColorIdle"] = { c_i.x, c_i.y, c_i.z, c_i.w };
	data["handleColorHover"] = { c_h.x, c_h.y, c_h.z, c_h.w };
	data["handleColorPressed"] = { c_p.x, c_p.y, c_p.z, c_p.w };
	if (!m_onValueChangedActionKey.empty())
		data["actionKey"] = m_onValueChangedActionKey;

	data["showValueText"] = m_showValueText;
	data["valueTextScale"] = m_valueTextScale;
	data["valueTextOffset"] = { m_valueTextOffset.x, m_valueTextOffset.y };

	DirectX::XMFLOAT4 tc;
	DirectX::XMStoreFloat4(&tc, m_valueTextColor);
	data["valueTextColor"] = { tc.x, tc.y, tc.z, tc.w };

	if (!m_onValueChangedActionKey.empty())
		data["actionKey"] = m_onValueChangedActionKey;

	return data;
}

void Slider::Deserialize(const nlohmann::json& jsonData)
{
	UIBase::Deserialize(jsonData);

	if (jsonData.contains("min")) m_min = jsonData["min"];
	if (jsonData.contains("max")) m_max = jsonData["max"];
	if (jsonData.contains("value")) {
		float val = jsonData["value"];
		SetValue(val);
	}

	std::string handleIdle = jsonData.value("handlePathIdle", "");
	std::string handleHover = jsonData.value("handlePathHover", "");
	std::string handlePressed = jsonData.value("handlePathPressed", "");

	if (handleIdle.empty() && jsonData.contains("textureHandle"))
	{
		handleIdle = jsonData["textureHandle"];
	}

	if (!handleIdle.empty() || !handleHover.empty() || !handlePressed.empty())
	{
		if (handleIdle.empty()) handleIdle = handleHover.empty() ? handlePressed : handleHover;
		SetHandleTextures(handleIdle, handleHover, handlePressed);
	}

	if (jsonData.contains("handleScaleIdle")) m_handleScaleIdle = jsonData["handleScaleIdle"];
	if (jsonData.contains("handleScaleHover")) m_handleScaleHover = jsonData["handleScaleHover"];
	if (jsonData.contains("handleScalePressed")) m_handleScalePressed = jsonData["handleScalePressed"];

	auto ReadColor = [&](const char* key, DirectX::XMVECTOR& target)
	{
		if (!jsonData.contains(key)) return;
		DirectX::XMFLOAT4 f;
		f.x = jsonData[key][0];
		f.y = jsonData[key][1];
		f.z = jsonData[key][2];
		f.w = jsonData[key][3];
		target = DirectX::XMLoadFloat4(&f);
	};

	ReadColor("handleColorIdle", m_handleColorIdle);
	ReadColor("handleColorHover", m_handleColorHover);
	ReadColor("handleColorPressed", m_handleColorPressed);

	m_showValueText = jsonData.value("showValueText", m_showValueText);
	m_valueTextScale = jsonData.value("valueTextScale", m_valueTextScale);

	if (jsonData.contains("valueTextOffset"))
	{
		m_valueTextOffset.x = jsonData["valueTextOffset"][0];
		m_valueTextOffset.y = jsonData["valueTextOffset"][1];
	}

	ReadColor("valueTextColor", m_valueTextColor);

	if (jsonData.contains("actionKey")) {m_onValueChangedActionKey = jsonData["actionKey"];	}
}
