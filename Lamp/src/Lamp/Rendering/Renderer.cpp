#include "lppch.h"
#include "Renderer.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;
	SpriteBatch* Renderer::m_pSpriteBatch = new SpriteBatch();

	void Renderer::Initialize()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_pSpriteBatch->Initialize();
	}
	void Renderer::Begin(OrthographicCamera & camera)
	{
		s_pSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		m_pSpriteBatch->Begin();
	}

	void Renderer::End()
	{
		m_pSpriteBatch->End();
		m_pSpriteBatch->RenderBatches();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Renderer::Draw(const std::shared_ptr<Shader>& shader, IEntity* pEntity)
	{
		shader->Bind();
		shader->UploadUniformInt("textureSampler", 0);
		shader->UploadUniformMat4("P", s_pSceneData->ViewProjectionMatrix);

		m_pSpriteBatch->Draw(pEntity);
	}
}