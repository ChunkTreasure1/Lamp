#pragma once

#include <glm/glm.hpp>
#include "Lamp/Rendering/Vertices/VertexArray.h"

namespace Lamp
{
	struct RendererCapabilities
	{
	public:
		uint32_t MaxTextureSlots = 0;
	};

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			DX11 = 2
		};

	public:
		virtual ~RendererAPI() = default;
		virtual void Initialize() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t = 0) = 0;
		virtual void DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;

		const RendererCapabilities& GetRendererCapabilities() { return m_RendererCapabilities; }

	protected:
		RendererCapabilities m_RendererCapabilities;
	
	public:
		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();

	private:
		static API s_API;
	};
}