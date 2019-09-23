#include "lppch.h"
#include "Renderer.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	Renderer::Renderer(Window* pWindow)
	{
		m_pWindow = pWindow;

		//Create the camera
		m_pCamera.reset(new Camera2D);
		
		//Setup shader
		m_pShader.reset(new Shader("Shaders/colorShading.vert", "Shaders/colorShading.frag"));

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		m_pSpriteBatch.reset(new SpriteBatch());
		m_pFPSLimiter.reset(new FPSLimiter(75.f));
	}

	Renderer::~Renderer()
	{}

	void Renderer::Draw()
	{
		m_pFPSLimiter->Begin();
		m_Time += 0.01f;
		m_pCamera->Update();

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
		glm::mat4 cameraMatrix = m_pCamera->GetMatrix();
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

		m_pWindow->Update();

		/////End drawing/////

		m_FPS = m_pFPSLimiter->End();

		//Print the FPS every 10 frames
		static uint32_t frameCount = 0;
		frameCount++;

		if (frameCount == 10)
		{
			std::cout << m_FPS << std::endl;
			frameCount = 0;
		}
	}
}