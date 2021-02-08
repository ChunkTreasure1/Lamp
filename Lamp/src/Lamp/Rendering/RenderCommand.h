#pragma once

#include "RendererAPI.h"

namespace Lamp
{
class RenderCommand
{
public:
    static void Initialize()
    {
        s_RendererAPI->Initialize();
    }

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        s_RendererAPI->SetViewport(x, y, width, height);
    }

    static void SetClearColor(const glm::vec4& color)
    {
        s_RendererAPI->SetClearColor(color);
    }

    static void Clear()
    {
        s_RendererAPI->Clear();
    }

    static void ClearDepth()
    {
        s_RendererAPI->ClearDepth();
    }

    static void OffsetPolygon(float factor = 0, float unit = 0)
    {
        s_RendererAPI->OffsetPolygon(factor, unit);
    }

    static void DrawIndexed(const Ref<VertexArray>& VertexArray, uint32_t count = 0)
    {
        s_RendererAPI->DrawIndexed(VertexArray, count);
    }

    static void DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
    {
        s_RendererAPI->DrawIndexedLines(vertexArray, count);
    }

    static const RendererCapabilities& GetCapabilities()
    {
        return s_RendererAPI->GetRendererCapabilities();
    }

private:
    static Scope<RendererAPI> s_RendererAPI;
};
}