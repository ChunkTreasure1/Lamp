#pragma once

#include "Lamp/Rendering//RendererAPI.h"

namespace Lamp
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual ~OpenGLRendererAPI() override = default;


		void Initialize() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void ClearColor() override;
		void ClearDepth() override;
		void OffsetPolygon(float factor, float unit) override;
		void SetCullFace(CullFace face) const override;
		void EnableBlending(bool state) const override;
		
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t = 0) override;
		void DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
	};
}