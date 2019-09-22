#include "Sprite.h"

namespace Lamp
{
	Sprite::Sprite(const glm::vec4 & destRect, const glm::vec4 & uvRect, GLuint texture, float depth, const Color & color)
	{
		m_Color = color;
		m_DestinationRectangle = destRect;
		m_UVRect = uvRect;

		m_Texture = texture;
		m_Depth = depth;
	}

	Sprite::Sprite(const glm::vec4 & destRect, GLuint texture, float depth)
	{
		m_DestinationRectangle = destRect;
		m_UVRect = glm::vec4(0, 0, 1, 1);
		m_Texture = texture;
		m_Depth = depth;

		Color color;
		color.R = 255;
		color.G = 255;
		
		color.B = 255;
		color.A = 255;
		m_Color = color;
	}

	Sprite::~Sprite()
	{
	}
}