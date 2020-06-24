#include "lppch.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;
	RendererCapabilities Renderer::s_RenderCapabilities;

	void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         LP_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       LP_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          LP_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: LP_CORE_TRACE(message); return;
		}

		LP_CORE_ASSERT(false, "Unknown severity level!");
	}

	void Renderer::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		//glEnable(GL_FRAMEBUFFER_SRGB);

		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &s_RenderCapabilities.MaxTextureSlots);
	
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(OpenGLMessageCallback, 0);

		Renderer3D::Initialize();
		Renderer2D::Initialize();
	}

	void Renderer::Shutdown()
	{
		Renderer3D::Shutdown();
		Renderer2D::Shutdown();
	}

	void Renderer::Begin(OrthographicCamera & camera)
	{
		s_pSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::End()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Renderer::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawIndexedLines(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}
}