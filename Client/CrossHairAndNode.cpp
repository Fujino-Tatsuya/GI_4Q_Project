#include "stdafx.h"
#include "Renderer.h"
#include "GameObjectBase.h"

#include "ResourceManager.h"
#include "TimeManager.h"
#include "SoundManager.h"
#include "InputManager.h"

#include "CrossHairAndNode.h"
#include "Player.h"

#include "Shared/Config/Option.h"

REGISTER_TYPE(CrossHairAndNode)

void CrossHairAndNode::Initialize()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	m_crosshairTextureAndOffset = resourceManager.GetTextureAndOffset("Cross_Hair_Middle.png");
	m_RhythmLineTextureAndOffset = resourceManager.GetTextureAndOffset("Line.png");
	m_NodeAndOffset = resourceManager.GetTextureAndOffset("Line.png");

	m_NodeDataPtr = SoundManager::GetInstance().GetNodeDataPtr();

	m_CrossHairSize = m_prevCrossHairSize = 0.04f;
	m_NodeStartSize = m_prevNodeStartSize = 0.104f;
	m_NodeEndSize = m_prevNodeEndSize = 0.176f;

	m_NodeStart = 0.1f;
	m_NodeEnd = 0.45f;

	m_linePos = 0.45f;
	m_lineScl = 0.04f;
}

void CrossHairAndNode::Update()
{
	float delta = TimeManager::GetInstance().GetDeltaTime();
	InputManager& input = InputManager::GetInstance();

	ResizeMiddleCH(input, delta);

	GenerateNode();

	for_each(m_UINode.begin(), m_UINode.end(), [&](auto& time) { time -= SoundManager::GetInstance().GetAudioDeltaTime(); });
	if (!m_UINode.empty() && m_UINode.front() < 0.0f)
	{
		m_UINode.pop_front();
	}
}

void CrossHairAndNode::Render()
{
	Renderer& renderer = Renderer::GetInstance();

	if (!dynamic_cast<Player*>(GetOwner())->GetActiveDeadEye())
	{
		RenderCrossHair(renderer);

		if (!m_UINode.empty()) 
			RenderUINode(renderer);
	}
}

#ifdef _DEBUG
void CrossHairAndNode::RenderImGui()
{
	ImGui::DragFloat("Middle Size", &m_CrossHairSize, 0.004f, 0.004f, 0.04f);
	ImGui::DragFloat("Start Size", &m_NodeStartSize, 0.004f, 0.004f, 1);
	ImGui::DragFloat("End Size", &m_NodeEndSize, 0.004f, 0.004f, 1);
	ImGui::DragFloat("Node start", &m_NodeStart, 0.001f, 0.1f, 0.5f);
	ImGui::DragFloat("Node End", &m_NodeEnd, 0.001f, 0.1f, 0.5f);
	ImGui::DragFloat("Interval", &m_linePos, 0.001f, 0.1f, 0.5f);
	ImGui::DragFloat("Line Scl", &m_lineScl, 0.001f, 0.004f, 0.5f);
}
#endif-

void CrossHairAndNode::Finalize()
{
	m_UINode.clear();
}

nlohmann::json CrossHairAndNode::Serialize()
{
	nlohmann::json jsonData;

	jsonData["CrossHairSize"] = m_CrossHairSize;

	jsonData["NodeStartSize"] = m_NodeStartSize;
	jsonData["NodeEndSize"] = m_NodeEndSize;

	jsonData["NodeStart"] = m_NodeStart;
	jsonData["NodeEnd"] = m_NodeEnd;

	jsonData["LinePos"] = m_linePos;
	jsonData["LineScl"] = m_lineScl;

	return jsonData;
}

void CrossHairAndNode::Deserialize(const nlohmann::json& jsonData)
{
	if (jsonData.contains("CrossHairSize"))
		m_CrossHairSize = jsonData["CrossHairSize"];

	if (jsonData.contains("NodeStartSize"))
		m_NodeStartSize = jsonData["NodeStartSize"];

	if (jsonData.contains("NodeEndSize"))
		m_NodeEndSize = jsonData["NodeEndSize"];

	if (jsonData.contains("NodeStart"))
		m_NodeStart = jsonData["NodeStart"];

	if (jsonData.contains("NodeEnd"))
		m_NodeEnd = jsonData["NodeEnd"];

	if (jsonData.contains("LinePos"))
		m_linePos = jsonData["LinePos"];

	if (jsonData.contains("LineScl"))
		m_lineScl = jsonData["LineScl"];
}


void CrossHairAndNode::RenderUINode(Renderer& renderer)
{
	auto nodes = m_UINode;

	const float nodeStart = m_NodeStart;
	const float nodeEnd = m_NodeEnd;
	const float startSize = m_NodeStartSize;
	const float endSize = m_NodeEndSize;

	auto tex = m_NodeAndOffset.first;
	auto off = m_NodeAndOffset.second;

	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[nodes, nodeStart, nodeEnd, startSize, endSize, tex, off]()
		{
			for (const auto& time : nodes)
			{
				float temp = time / SoundManager::GetInstance().GetRhythmOffset();

				float pos = std::clamp(std::lerp(nodeEnd, nodeStart, temp), 0.0f, nodeEnd);
				float scale = std::clamp(std::lerp(startSize, endSize, temp), 0.0f, 1.0f);

				Renderer::GetInstance().RenderImageNrmPosition(tex, { pos, 0.5f }, off, scale);
				Renderer::GetInstance().RenderImageNrmPosition(tex, { 1.0f - pos, 0.5f }, off, -scale);
			}
		}
	);
}

void CrossHairAndNode::ResizeMiddleCH(InputManager& input, float delta)
{
	if (input.GetKeyDown(KeyCode::LeftBracket))
	{
		m_CrossHairSize -= 1.0f * delta;

		if (m_CrossHairSize < 0.004f)
		{
			m_CrossHairSize = 0.004f;
		}
	}

	if (input.GetKeyDown(KeyCode::RightBracket))
	{
		m_CrossHairSize += 1.0f * delta;

		if (m_CrossHairSize > 0.04f)
		{
			m_CrossHairSize = 0.04f;
		}
	}
}

void CrossHairAndNode::RenderCrossHair(Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back([&]() { Renderer::GetInstance().RenderImageNrmPosition(m_crosshairTextureAndOffset.first, { 0.5f, 0.5f }, m_crosshairTextureAndOffset.second, m_CrossHairSize); });
	renderer.UI_RENDER_FUNCTIONS().emplace_back([&]() { Renderer::GetInstance().RenderImageNrmPosition(m_RhythmLineTextureAndOffset.first, { m_linePos, 0.5f }, m_RhythmLineTextureAndOffset.second, m_lineScl); });
	renderer.UI_RENDER_FUNCTIONS().emplace_back([&]() { Renderer::GetInstance().RenderImageNrmPosition(m_RhythmLineTextureAndOffset.first, { 1-m_linePos, 0.5f }, m_RhythmLineTextureAndOffset.second, m_lineScl); });
}

void CrossHairAndNode::GenerateNode()
{
	SoundManager& sm = SoundManager::GetInstance();

	/*if (!sm.ConsumeNodeChanged())
		return;*/
	sm.AddNodeGeneratedListenerOnce([&]() { m_UINode.push_back(sm.GetRhythmOffset() - Config::visualLead); });

}
