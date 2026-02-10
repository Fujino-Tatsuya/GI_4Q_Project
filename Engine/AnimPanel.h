#pragma once

#include "Panel.h"

class AnimPanel : public Panel
{
	friend class SceneBase;

public:
	AnimPanel();

	std::string GetTypeName() const override { return "AnimPanel"; }
	void RenderUI(class Renderer& renderer) override;

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& data) override;

private:
	void Update();
	void UpdateFlipBookRect();
	void GetTextureResolution(int& outWidth, int& outHeight) const;
	int GetMaxFrames() const;
	void ClampFrame();

	RECT m_UIAnimationRect = {};
	int m_rows = 1;
	int m_columns = 1;
	int m_startFrame = 0;
	int m_endFrame = 1;
	float m_framesPerSecond = 8.0f;
	bool m_loop = true;
	bool m_playing = true;
	bool auto_play_ = true;
	bool destroy_on_finish_ = false;
	float playback_speed_ = 1.0f;
	int m_currentFrame = 0;
	float m_accumulatedTime = 0.0f;
};
