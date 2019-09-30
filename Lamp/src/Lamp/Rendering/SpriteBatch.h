#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include <LampEntity/Entity/Entity.h>

#include "Vertex.h"
#include "Lamp/Rendering/Sprite.h"

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
		void Initialize();
		void Begin(GlyphSortType sortType = GlyphSortType::TEXTURE);
		void End();

		void Draw(const std::unique_ptr<LampEntity::IEntity>& pEntity);
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
		uint32_t m_VBO;
		uint32_t m_VAO;
		GlyphSortType m_SortType;

		std::vector<Glyph*> m_Glyphs;
		std::vector<RenderBatch> m_RenderBatches;
	};
}