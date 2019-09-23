#include "lppch.h"
#include "Renderer.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	Renderer::Renderer()
		: m_CameraController(1280.f / 720.f)
	{

		//Setup shader
		m_pShader.reset(new Shader("Shaders/colorShading.vert", "Shaders/colorShading.frag"));

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		m_pSpriteBatch.reset(new SpriteBatch());
	}

	Renderer::~Renderer()
	{}

	void Renderer::Draw(Timestep ts)
	{
		m_CameraController.Update(ts);

		/////Draw Game /////

		//Clear the screen
		glClearDepth(1.0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind the shader
		m_pShader->Bind();

		glActiveTexture(GL_TEXTURE0);

		//Set the texture sampler
		GLint textureLocation = m_pShader->GetUniformLocation("textureSampler");
		glUniform1i(textureLocation, 0);

		//Set the camera matrix
		GLuint location = m_pShader->GetUniformLocation("P");
		glm::mat4 cameraMatrix = m_CameraController.GetCamera().GetViewProjectionMatrix();
		glUniformMatrix4fv(location, 1, GL_FALSE, &(cameraMatrix[0][0]));

		//Start drawing
		m_pSpriteBatch->Begin();

		for (int i = 0; i < m_RenderSprites.size(); i++)
		{
			m_pSpriteBatch->Draw(m_RenderSprites[i]);
		}

		//End drawing
		m_pSpriteBatch->End();
		m_pSpriteBatch->RenderBatches();

		glBindTexture(GL_TEXTURE_2D, 0);
		m_pShader->Unbind();

		/////End drawing/////
	}

	void Renderer::OnEvent(Event & e)
	{
		m_CameraController.OnEvent(e);
	}
}