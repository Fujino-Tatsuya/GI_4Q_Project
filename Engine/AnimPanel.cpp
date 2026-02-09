#include "stdafx.h"
#include "AnimPanel.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "TimeManager.h"

AnimPanel::AnimPanel()
{
	UpdateFlipBookRect();
}

void AnimPanel::RenderUI(Renderer& renderer)
{
	if (!IsActuallyActive())
		return;

	Update();

	auto tex = m_textureIdle.first;
	auto pos = GetWorldPosition();
	auto offset = m_textureIdle.second;
	auto scale = GetFinalScale();
	auto color = m_colorIdle;
	auto depth = m_depth;

	const bool useAnimRect = (m_rows > 1 || m_columns > 1);
	const RECT* srcRect = useAnimRect ? &m_UIAnimationRect : nullptr;

	renderer.UI_RENDER_FUNCTIONS().emplace_back(
		[tex, pos, offset, scale, color, depth, srcRect]()
		{
			Renderer::GetInstance().RenderImageUIPosition(
				tex, pos, offset, scale, color, depth, srcRect
			);
		});

	for (auto& child : m_children)
	{
		if (child->GetActive())
			child->RenderUI(renderer);
	}
}

nlohmann::json AnimPanel::Serialize() const
{
	nlohmann::json data = UIBase::Serialize();

	data["animRows"] = m_rows;
	data["animCols"] = m_columns;
	data["animFPS"] = m_framesPerSecond;
	data["animLoop"] = m_loop;
	data["animAutoPlay"] = auto_play_;
	data["animStartFrame"] = m_startFrame;
	data["animEndFrame"] = m_endFrame;
	data["animPlaybackSpeed"] = playback_speed_;
	data["animDestroyOnFinish"] = destroy_on_finish_;

	return data;
}

void AnimPanel::Deserialize(const nlohmann::json& data)
{
	UIBase::Deserialize(data);

	if (data.contains("animRows")) m_rows = data["animRows"];
	if (data.contains("animCols")) m_columns = data["animCols"];
	if (data.contains("animFPS")) m_framesPerSecond = data["animFPS"];
	if (data.contains("animLoop")) m_loop = data["animLoop"];
	if (data.contains("animAutoPlay")) auto_play_ = data["animAutoPlay"];
	if (data.contains("animStartFrame")) m_startFrame = data["animStartFrame"];
	if (data.contains("animEndFrame")) m_endFrame = data["animEndFrame"];
	if (data.contains("animPlaybackSpeed")) playback_speed_ = data["animPlaybackSpeed"];
	if (data.contains("animDestroyOnFinish")) destroy_on_finish_ = data["animDestroyOnFinish"];

	m_playing = auto_play_;
	m_currentFrame = 0;
	m_accumulatedTime = 0.0f;

	UpdateFlipBookRect();
}

void AnimPanel::Update()
{
	if (!m_playing) return;
	if (m_rows <= 0 || m_columns <= 0) return;

	const int maxFrames = GetMaxFrames();
	if (maxFrames <= 1) return;
	if (m_framesPerSecond <= 0.0f) return;
	if (playback_speed_ <= 0.0f) return;

	const float dt = TimeManager::GetInstance().GetDeltaTime();

	m_accumulatedTime += dt * playback_speed_;
	const float frameDuration = 1.0f / m_framesPerSecond;

	bool updated = false;
	while (m_accumulatedTime >= frameDuration) {
		m_accumulatedTime -= frameDuration;
		++m_currentFrame;
		updated = true;

		if (m_currentFrame >= maxFrames) {
			if (m_loop) {
				m_currentFrame = 0;
			} else {
				m_currentFrame = maxFrames - 1;
				m_playing = false;
				if (destroy_on_finish_) {
					SetActive(false);
				}
				break;
			}
		}
	}

	if (updated) {
		UpdateFlipBookRect();
	}
}

void AnimPanel::UpdateFlipBookRect()
{
	if (!m_textureIdle.first) {
		m_UIAnimationRect = { 0, 0, 0, 0 };
		return;
	}

	int texWidth = 0, texHeight = 0;
	GetTextureResolution(texWidth, texHeight);

	if (m_columns <= 0) m_columns = 1;
	if (m_rows <= 0) m_rows = 1;

	ClampFrame();

	const int frameWidth = texWidth / m_columns;
	const int frameHeight = texHeight / m_rows;

	const int currentAbsoluteFrame = m_currentFrame;
	const int colIndex = currentAbsoluteFrame % m_columns;
	const int rowIndex = currentAbsoluteFrame / m_columns;

	m_UIAnimationRect.left = colIndex * frameWidth;
	m_UIAnimationRect.top = rowIndex * frameHeight;
	m_UIAnimationRect.right = m_UIAnimationRect.left + frameWidth;
	m_UIAnimationRect.bottom = m_UIAnimationRect.top + frameHeight;
}

void AnimPanel::GetTextureResolution(int& outWidth, int& outHeight) const
{
	outWidth = 0;
	outHeight = 0;

	if (m_textureIdle.first) {
		com_ptr<ID3D11Resource> resource;
		m_textureIdle.first->GetResource(resource.GetAddressOf());

		if (resource) {
			com_ptr<ID3D11Texture2D> texture2D;
			if (SUCCEEDED(resource.As(&texture2D))) {
				D3D11_TEXTURE2D_DESC desc;
				texture2D->GetDesc(&desc);
				outWidth = static_cast<int>(desc.Width);
				outHeight = static_cast<int>(desc.Height);
			}
		}
	}
}

int AnimPanel::GetMaxFrames() const
{
	if (m_rows <= 0 || m_columns <= 0) return 0;
	const int gridFrames = m_rows * m_columns;

	int availableFrames = gridFrames;

	if (m_endFrame > 0 && m_endFrame <= gridFrames) {
		availableFrames = m_endFrame;
	}

	if (m_startFrame > 0 && m_startFrame < availableFrames) {
		return availableFrames - m_startFrame;
	}

	return availableFrames;
}

void AnimPanel::ClampFrame()
{
	if (m_rows <= 0) m_rows = 1;
	if (m_columns <= 0) m_columns = 1;

	const int maxFrames = GetMaxFrames();

	if (maxFrames <= 0) {
		m_currentFrame = 0;
	} else {
		if (m_currentFrame < 0) m_currentFrame = 0;
		if (m_currentFrame >= maxFrames) m_currentFrame = maxFrames - 1;
	}
}
