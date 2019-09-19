#include "lppch.h"
#include "Application.h"

#include "Lamp/Core.h"
#include "GL/glew.h"

namespace Lamp 
{
	//Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		m_pWindow = new Window();

		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f,
		};

		glGenBuffers(1, &m_VertexBuffer);

		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			void main() 
			{
				gl_Position.xyz = a_Position;
				gl_Position.w = 1.0;
			}
		)";

		std::string fragementSrc = R"(
			#version 330 core

			out vec3 color;
			
			void main() 
			{
				color = vec3(1, 0, 0);
			}
		)";

		m_pShader = new Shader(fragementSrc, vertexSrc);
	}

	Application::~Application()
	{
		m_Running = false;
		delete m_pShader;
		delete m_pWindow;
	}

	void Application::Run()
	{

		while (m_Running)
		{
			if (glfwWindowShouldClose(m_pWindow->GetNativeWindow()))
			{
				m_Running = false;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_pShader->Bind();

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, 3);
			glDisableVertexAttribArray(0);

			glfwSwapBuffers(m_pWindow->GetNativeWindow());
			glfwPollEvents();
		}

		glfwTerminate();
	}
}