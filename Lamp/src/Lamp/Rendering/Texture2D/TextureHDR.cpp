#include "lppch.h"
#include "TextureHDR.h"

#include "Lamp/Rendering/Texture2D/TextureCache.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	TextureHDR::TextureHDR(const std::string& path)
	{
		glGenFramebuffers(1, &m_FBO);
		glGenRenderbuffers(1, &m_RBO);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glBindRenderbuffer(GL_FRAMEBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		auto tex = TextureCache::GetTexture(path);
		m_HdrMapId = std::get<0>(tex);
		m_Width = std::get<1>(tex);
		m_Height = std::get<2>(tex);
		m_InternalFormat = std::get<3>(tex);
		m_DataFormat = std::get<4>(tex);

		glGenTextures(1, &m_CubeMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
		glm::mat4 views[] =
		{
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
			glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f)),
		};
	}
}