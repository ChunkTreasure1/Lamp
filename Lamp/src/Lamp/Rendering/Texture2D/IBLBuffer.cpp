#include "lppch.h"
#include "IBLBuffer.h"

#include <glad/glad.h>
#include <stb/stb_image.h>
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include "Lamp/Rendering/Renderer3D.h"

namespace Lamp
{
	IBLBuffer::IBLBuffer(const std::string& path)
	{
		m_EqCubeShader = ShaderLibrary::GetShader("EqCube");
		m_ConvolutionShader = ShaderLibrary::GetShader("Convolution");
		m_PrefilterShader = ShaderLibrary::GetShader("Prefilter");
		m_BRDFShader = ShaderLibrary::GetShader("BRDFIntegrate");

		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glGenTextures(1, &m_HdrTextureId);
			glBindTexture(GL_TEXTURE_2D, m_HdrTextureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load HDR image." << std::endl;
		}

		glGenFramebuffers(1, &m_RendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

		glGenTextures(1, &m_CubeMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		m_CaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
		m_CaptureViews =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};


		//Render to cubemap
		m_EqCubeShader->Bind();
		m_EqCubeShader->UploadInt("u_EquirectangularMap", 0);
		m_EqCubeShader->UploadMat4("u_Projection", m_CaptureProjection);
		glBindTextureUnit(0, m_HdrTextureId);

		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_EqCubeShader->UploadMat4("u_View", m_CaptureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_CubeMapId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Renderer3D::DrawCube();
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		////////////////////////

		/////Convolute map//////
		glGenTextures(1, &m_IrradianceId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceId);
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

		m_ConvolutionShader->Bind();
		m_ConvolutionShader->UploadInt("u_EnvironmentMap", 0);
		m_ConvolutionShader->UploadMat4("u_Projection", m_CaptureProjection);
		glBindTextureUnit(0, m_CubeMapId);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		for (int i = 0; i < 6; i++)
		{
			m_ConvolutionShader->UploadMat4("u_View", m_CaptureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Renderer3D::DrawCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		////////////////////////////

		/////Prefilter Map/////
		glGenTextures(1, &m_PrefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		m_PrefilterShader->Bind();
		m_PrefilterShader->UploadInt("u_EnvironmentMap", 0);
		m_PrefilterShader->UploadMat4("u_Projection", m_CaptureProjection);

		glBindTextureUnit(0, m_CubeMapId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		
		uint32_t maxMips = 5;
		for (uint32_t i = 0; i < maxMips; i++)
		{
			uint32_t mipWidth = 128 * std::pow(0.5f, i);
			uint32_t mipHeight = 128 * std::pow(0.5f, i);

			glViewport(0, 0, mipWidth, mipHeight);
			float roughness = (float)i / (float)(maxMips - 1);
			m_PrefilterShader->UploadFloat("u_Roughness", roughness);

			for (uint32_t i = 0; i < 6; i++)
			{
				m_PrefilterShader->UploadMat4("u_View", m_CaptureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, i);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				Renderer3D::DrawCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		///////////////////////

		/////BRDF LUT/////
		glGenTextures(1, &m_BRDFLUTMap);
		glBindTexture(GL_TEXTURE_2D, m_BRDFLUTMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTMap, 0);
		glViewport(0, 0, 512, 512);

		m_BRDFShader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer3D::DrawQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//////////////////
	}

	void IBLBuffer::Bind()
	{
		glDepthFunc(GL_LEQUAL);
		glBindTextureUnit(0, m_CubeMapId);
	}
}