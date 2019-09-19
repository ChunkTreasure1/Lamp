#pragma once

#include <glm/glm.hpp>
#include <Rendering/Textures/SpriteBatch.h>
#include <glm/glm.hpp>

class Bullet
{
public:
	Bullet(glm::vec2 pos, glm::vec2 dir, float speed, int lifeTime);
	~Bullet();

	void Draw(CactusEngine::SpriteBatch& spriteBatch);
	bool Update();

private:

	//Member vars
	int m_LifeTime;
	float m_Speed;
	glm::vec2 m_Direction;
	glm::vec2 m_Position;
};

