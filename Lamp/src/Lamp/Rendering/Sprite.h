#pragma once

#include <GL/glew.h>
#include "Lamp/Rendering/Texture/GLTexture.h"

#include <string>

namespace Lamp
{
	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		//Functions
		void Draw();
		void Init(float x, float y, float width, float height, std::string texturePath);

	private:

		//Member vars
		float m_X = 0;
		float m_Y = 0;

		float m_Width = 0;
		float m_Height = 0;

		GLuint m_VboID;
		GLTexture m_Texture;
	};
}