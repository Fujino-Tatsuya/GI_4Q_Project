#pragma once
#include "Text.h"

class Slider : public UIBase
{
public:
	Slider();
	~Slider() override = default;

	void RenderUI(Renderer& renderer) override;
	bool CheckInput(const POINT& mousePos, bool isMousePressed);

	void SetRange(float min, float max);
	void SetValue(float newValue);
	float& GetValue() { return m_realvalue; }
	float& GetMin() { return m_min; }
	float& GetMax() { return m_max; }

	void AttachText(Text* textBox) { m_valueText = textBox; }

	void SetHandleTexture(const std::string& tex);
	void SetHandleTextures(const std::string& idle, const std::string& hover, const std::string& pressed);
	void SetHandleTextureJSON(const std::string& path);

	void AddListener(std::function<void(float)>);
	void NotifyValueChanged();

	std::string GetTypeName() const override { return "Slider"; }

	void SetActionKey(const std::string& key) { m_onValueChangedActionKey = key; }
	std::string GetActionKey() const { return m_onValueChangedActionKey; }

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& jsonData) override;

public:
	std::string m_handlePathIdle = "UI_IDLE.png";
	std::string m_handlePathHover = "UI_IDLE.png";
	std::string m_handlePathPressed = "UI_IDLE.png";

	float m_handleScaleIdle = 1.0f;
	float m_handleScaleHover = 1.0f;
	float m_handleScalePressed = 1.0f;

	DirectX::XMVECTOR m_handleColorIdle = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMVECTOR m_handleColorHover = { 0.9f, 0.9f, 0.9f, 1.0f };
	DirectX::XMVECTOR m_handleColorPressed = { 0.7f, 0.7f, 0.7f, 1.0f };

	Text* m_valueText = nullptr;

	bool m_showValueText = true;
	float m_valueTextScale = 0.21f;
	DirectX::XMFLOAT2 m_valueTextOffset = { -2.0f, -16.0f };
	DirectX::XMVECTOR m_valueTextColor = { powf(0.15686274f, 2.2f),
	powf(0.07843137f, 2.2f),
	powf(0.05490196f, 2.2f), 1 };

	DirectX::XMFLOAT2 m_cachedHandleCenter = { 0.f, 0.f };

private:
	enum class HandleState
	{
		Idle,
		Hover,
		Pressed,
		Done
	};

	void UpdateRect() override;
	void UpdateHandleRect();

	float m_min = 0.0f;
	float m_max = 1.0f;
	float m_rendervalue = 1.0f;
	float m_realvalue = 1.0f;
	bool m_dragging = false;

	HandleState m_handleState = HandleState::Idle;
	RECT m_handleRect = {};

	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_handleTexIdle{};
	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_handleTexHover{};
	std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> m_handleTexPressed{};

	std::vector<std::function<void(float)>> listeners;
	std::string m_onValueChangedActionKey = "";

};
