#include <GL/glew.h>
#include <cstddef>

#include "Sprite.h"
#include "Vertex.h"
#include "../Systems/ResourceManager.h"

namespace CactusEngine
{
	Sprite::Sprite()
	{
		m_VboID = 0;
	}


	Sprite::~Sprite()
	{
		if (m_VboID != 0)
		{
			glDeleteBuffers(1, &m_VboID);
		}
	}

	//Draws the sprite
	void Sprite::Draw()
	{
		glBindTexture(GL_TEXTURE_2D, m_Texture.Id);

		//Bind the buffer and enable the buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
		glEnableVertexAttribArray(0);

		//Setup the draw data
		//Position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		//Color attribute pointer
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
		//UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Disable and unbind the buffer
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//Initializes the sprite
	void Sprite::Init(float x, float y, float width, float height, std::string texturePath)
	{
		//Set the variables
		m_X = x;
		m_Y = y;

		m_Width = width;
		m_Height = height;

		m_Texture = ResourceManager::GetTexture(texturePath);

		//Check if the vbo has been created
		if (m_VboID == 0)
		{
			glGenBuffers(1, &m_VboID);
		}

		Vertex vertexData[6];

		//First triangle
		vertexData[0].SetPos(x + width, y + height);
		vertexData[0].SetUV(1.0f, 1.0f);

		vertexData[1].SetPos(x, y + height);
		vertexData[1].SetUV(0, 1.0f);

		vertexData[2].SetPos(x, y);
		vertexData[2].SetUV(0, 0);

		//Second triangle
		vertexData[3].SetPos(x, y);
		vertexData[3].SetUV(0, 0);

		vertexData[4].SetPos(x + width, y);
		vertexData[4].SetUV(1.0f, 0);

		vertexData[5].SetPos(x + width, y + height);
		vertexData[5].SetUV(1.0f, 1.0f);

		//Set the color
		for (int i = 0; i < 6; i++)
		{
			vertexData[i].SetColor(255, 0, 255, 255);
		}

		vertexData[1].SetColor(0, 0, 255, 255);
		vertexData[3].SetColor(0, 255, 0, 255);

		//Bind, send and unbind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
