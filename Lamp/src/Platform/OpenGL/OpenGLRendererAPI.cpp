#include "lppch.h"
#include "OpenGLRendererAPI.h"
#include "Lamp/Core/Log.h"

#include <glad/glad.h>

namespace Lamp
{
	void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:			LP_CORE_CRITICAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:			LP_CORE_ERROR(message); return;
		case GL_DEBUG_SEVERITY_LOW:				LP_CORE_WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION:	LP_CORE_TRACE(message); return;
		}
	}

	void OpenGLRendererAPI::Initialize()
	{
#ifdef LP_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glDisable(GL_MULTISAMPLE);

		GLint maxTextures;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);

		m_RendererCapabilities.MaxTextureSlots = maxTextures;
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::OffsetPolygon(float factor, float unit)
	{
		if (factor == 0 || unit == 0)
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
		else
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(factor, unit);
		}
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		if (count == 0)
		{
			return;
		}

		uint32_t c = count ? count : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		if (indexCount == 0)
		{
			return;
		}

		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);
	}
}