#include "lppch.h"
#include "SpriteBatch.h"

namespace Lamp
{
	SpriteBatch::SpriteBatch() : m_VBO(0), m_VAO(0)
	{
	}

	SpriteBatch::~SpriteBatch()
	{}

	void SpriteBatch::Initialize()
	{
		//Creates the vertex array
		CreateVertexArray();
	}

	//Begins the sprite batch
	void SpriteBatch::Begin(GlyphSortType sortType)
	{
		m_SortType = sortType;
		m_RenderBatches.clear();

		for (int i = 0; i < m_Glyphs.size(); i++)
		{
			delete m_Glyphs[i];
		}
		m_Glyphs.clear();
	}

	//Ends the sprite batch
	void SpriteBatch::End()
	{
		SortGlyphs();
		CreateRenderBatches();
	}

	//Draws the a sprite
	void SpriteBatch::Draw(const Sprite& sprite)
	{
		Glyph *newGlyph = new Glyph;
		newGlyph->Texture = sprite.GetTexuture();
		newGlyph->Depth = sprite.GetDepth();

		newGlyph->TopLeft.Color = sprite.GetColor();
		newGlyph->TopLeft.SetPos(sprite.GetDestRect().x, sprite.GetDestRect().y + sprite.GetDestRect().w);
		newGlyph->TopLeft.SetUV(sprite.GetUVRect().x, sprite.GetUVRect().y + sprite.GetUVRect().w);

		newGlyph->BottomLeft.Color = sprite.GetColor();
		newGlyph->BottomLeft.SetPos(sprite.GetDestRect().x, sprite.GetDestRect().y);
		newGlyph->BottomLeft.SetUV(sprite.GetUVRect().x, sprite.GetUVRect().y);

		newGlyph->BottomRight.Color = sprite.GetColor();
		newGlyph->BottomRight.SetPos(sprite.GetDestRect().x + sprite.GetDestRect().z, sprite.GetDestRect().y);
		newGlyph->BottomRight.SetUV(sprite.GetUVRect().x + sprite.GetUVRect().z, sprite.GetUVRect().y);

		newGlyph->TopRight.Color = sprite.GetColor();
		newGlyph->TopRight.SetPos(sprite.GetDestRect().x + sprite.GetDestRect().z, sprite.GetDestRect().y + sprite.GetDestRect().w);
		newGlyph->TopRight.SetUV(sprite.GetUVRect().x + sprite.GetUVRect().w, sprite.GetUVRect().y + sprite.GetUVRect().w);

		m_Glyphs.push_back(newGlyph);
	}

	//Renders the sprites
	void SpriteBatch::RenderBatches()
	{
		glBindVertexArray(m_VAO);

		for (int i = 0; i < m_RenderBatches.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_RenderBatches[i].Texture);
			glDrawArrays(GL_TRIANGLES, m_RenderBatches[i].Offset, m_RenderBatches[i].NumVertices);
		}

		glBindVertexArray(0);
	}

	//Create the render batches
	void SpriteBatch::CreateRenderBatches()
	{
		//Create vector to hold vertices
		std::vector<Vertex> vertices;
		vertices.resize(m_Glyphs.size() * 6);

		//Return if the vector is empty
		if (m_Glyphs.empty())
		{
			return;
		}
		
		//Create a new render batch
		int offset = 0;
		int currentVertex = 0;
		m_RenderBatches.emplace_back(offset, 6, m_Glyphs[0]->Texture);

		vertices[currentVertex++] = m_Glyphs[0]->TopLeft;
		vertices[currentVertex++] = m_Glyphs[0]->BottomLeft;
		vertices[currentVertex++] = m_Glyphs[0]->BottomRight;

		vertices[currentVertex++] = m_Glyphs[0]->BottomRight;
		vertices[currentVertex++] = m_Glyphs[0]->TopRight;
		vertices[currentVertex++] = m_Glyphs[0]->TopLeft;

		offset += 6;

		for (int i = 1; i < m_Glyphs.size(); i++)
		{
			if (m_Glyphs[i]->Texture != m_Glyphs[i - 1]->Texture)
			{
				m_RenderBatches.emplace_back(offset, 6, m_Glyphs[i]->Texture);
			}
			else
			{
				m_RenderBatches.back().NumVertices += 6;
			}

			vertices[currentVertex++] = m_Glyphs[i]->TopLeft;
			vertices[currentVertex++] = m_Glyphs[i]->BottomLeft;
			vertices[currentVertex++] = m_Glyphs[i]->BottomRight;

			vertices[currentVertex++] = m_Glyphs[i]->BottomRight;
			vertices[currentVertex++] = m_Glyphs[i]->TopRight;
			vertices[currentVertex++] = m_Glyphs[i]->TopLeft;

			offset += 6;
		}

		//Bind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		//Orphan the buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		//Upload the buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//Creates an array of vertecies
	void SpriteBatch::CreateVertexArray()
	{
		//Check that the vertex array and buffer haven't been generated yet
		//Then generate them
		if (m_VAO == 0)
		{
			glGenVertexArrays(1, &m_VAO);
		}
		glBindVertexArray(m_VAO);

		if (m_VBO == 0)
		{
			glGenBuffers(1, &m_VBO);
		}

		//Bind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		//Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		//Setup the draw data
		//Position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		//Color attribute pointer
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
		//UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));

		glBindVertexArray(0);
	}

	//Sorts the glyphs into batches
	void SpriteBatch::SortGlyphs()
	{
		switch (m_SortType)
		{
		case Lamp::GlyphSortType::FRONT_TO_BACK:
			std::stable_sort(m_Glyphs.begin(), m_Glyphs.end(), FrontToBackSort);
			break;
		case Lamp::GlyphSortType::BACK_TO_FRONT:
			std::stable_sort(m_Glyphs.begin(), m_Glyphs.end(), BackToFrontSort);
			break;
		case Lamp::GlyphSortType::TEXTURE:
			std::stable_sort(m_Glyphs.begin(), m_Glyphs.end(), TextureSort);
			break;
		}
	}

	//Used when sorting front to back
	bool SpriteBatch::FrontToBackSort(Glyph* a, Glyph* b)
	{
		return (a->Depth < b->Depth);
	}

	//Used when sorting back to front
	bool SpriteBatch::BackToFrontSort(Glyph* a, Glyph* b)
	{
		return (a->Depth > b->Depth);
	}

	//Used when sorting by texture
	bool SpriteBatch::TextureSort(Glyph* a, Glyph* b)
	{
		return (a->Texture < b->Texture);
	}
}
