#include "lppch.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;

	void Renderer::Initialize()
	{
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glEnable(GL_MULTISAMPLE);

		//glEnable(GL_DEPTH_TEST);
		Renderer2D::Initialize();
		Renderer3D::Initialize();

	}

	void Renderer::Shutdown()
	{
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

	void Renderer::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawIndexedLines(const std::shared_ptr<VertexArray>& vertexArray)
	{
		glDrawElements(GL_LINES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}
}