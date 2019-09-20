#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "Vertex.h"

namespace Lamp
{
	enum class GlyphSortType
	{
		NONE,
		FRONT_TO_BACK,
		BACK_TO_FRONT,
		TEXTURE
	};

	struct Glyph
	{
		GLuint Texture;
		float Depth;

		Vertex TopLeft;
		Vertex BottomLeft;
		Vertex TopRight;
		Vertex BottomRight;
	};

	class RenderBatch
	{
	public:
		RenderBatch(GLuint offset, GLuint numVertices, GLuint texture) :
			Offset(offset),
			NumVertices(numVertices),
			Texture(texture)
		{

		}

		GLuint Offset;
		GLuint NumVertices;
		GLuint Texture;
	};

	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		//Functions
		void Begin(GlyphSortType sortType = GlyphSortType::TEXTURE);
		void End();

		void Draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color);
		void RenderBatches();

	private:

		//Member functions
		void CreateRenderBatches();
		void CreateVertexArray();
		void SortGlyphs();

		static bool FrontToBackSort(Glyph *a, Glyph *b);
		static bool BackToFrontSort(Glyph *a, Glyph *b);
		static bool TextureSort(Glyph *a, Glyph *b);

		//Member vars
		GLuint m_VBO;
		GLuint m_VAO;
		GlyphSortType m_SortType;

		std::vector<Glyph*> m_Glyphs;
		std::vector<RenderBatch> m_RenderBatches;
	};
}