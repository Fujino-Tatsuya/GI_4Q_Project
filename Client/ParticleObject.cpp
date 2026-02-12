#include "stdafx.h"
#include "ParticleObject.h"

#include "TimeManager.h"

REGISTER_TYPE(ParticleObject)

void ParticleObject::Initialize()
{
	SetIgnoreParentTransform(true);
}

void ParticleObject::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	//MoveDirection((1.0f / m_lifetime) * deltaTime * 0.5f, Direction::Up);

	m_lifetime -= deltaTime;
	if (m_lifetime <= 0.0f) SetAlive(false);
}