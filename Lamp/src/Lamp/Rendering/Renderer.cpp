#include "lppch.h"
#include "Renderer.h"

#include "Lamp/Entity/BaseComponents/SpriteComponent.h"
#include "Renderer2D.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;

	void Renderer::Initialize()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Renderer2D::Initialize();

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
		vertexArray->Bind();

		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}