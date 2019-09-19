#include "Bullet.h"
#include <Rendering/Textures/GLTexture.h>
#include <Rendering/Systems/ResourceManager.h>

Bullet::Bullet(glm::vec2 pos, glm::vec2 dir, float speed, int lifeTime)
{
	m_Position = pos;
	m_Direction = dir;
	m_Speed = speed;
	m_LifeTime = lifeTime;
}

Bullet::~Bullet()
{
}

//Draws the bullet
void Bullet::Draw(CactusEngine::SpriteBatch& spriteBatch)
{
	static CactusEngine::GLTexture texture = CactusEngine::ResourceManager::GetTexture("Textures/ff.png");
	glm::vec4 uv(0.f, 0.f, 1.f, 1.f);
	CactusEngine::Color color;

	color.R = 255;
	color.G = 255;
	color.B = 255;
	color.A = 255;

	glm::vec4 posSize = glm::vec4(m_Position.x, m_Position.y, 30, 30);
	spriteBatch.Draw(posSize, uv, texture.Id, 0.f, color);
}

//Updates the bullet
bool Bullet::Update()
{
	m_Position += m_Direction * m_Speed;
	m_LifeTime--;

	if (m_LifeTime == 0)
	{
		return true;
	}

	return false;
}
