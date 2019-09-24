#pragma once
#include "Lamp/Rendering/Vertex.h"
#include "glm/glm.hpp"

namespace Lamp
{
	class Sprite
	{
	public:
		Sprite(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color & color);
		Sprite(const glm::vec4& destRect, GLuint texture, float depth);
		~Sprite();

		//Getting
		inline const Color GetColor() const { return m_Color; }
		inline const glm::vec4 GetDestRect() const { return m_DestinationRectangle; }
		inline const glm::vec4 GetUVRect() const { return m_UVRect; }

		inline const GLuint GetTexuture() const { return m_Texture; }
		inline const float GetDepth() const { return m_Depth; }
		inline const uint32_t GetPosition() const { return m_Position; }

		//Setting
		inline void SetPosition(uint32_t pos) { m_Position = pos; }
		inline void SetDestRect(const glm::vec4& destRect) { m_DestinationRectangle = destRect; }
		inline void SetTexture(GLuint texture) { m_Texture = texture; }

	private:
		Color m_Color;
		glm::vec4 m_DestinationRectangle;
		glm::vec4 m_UVRect;

		GLuint m_Texture;
		float m_Depth = 0.f;
		uint32_t m_Position;
	};
}