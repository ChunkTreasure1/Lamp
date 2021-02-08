#include "ReflectionBuffer.h"
#include "lppch.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp {
ReflectionBuffer::ReflectionBuffer() {
  m_Projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f);
  m_Views = {
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f))};

  Generate();
}

void ReflectionBuffer::Generate() {
  if (m_RendererId) {
    glDeleteFramebuffers(1, &m_RendererId);
    glDeleteTextures(1, &m_TextureId);
  }

  glGenFramebuffers(1, &m_RendererId);

  glGenTextures(1, &m_TextureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Width,
                 m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  uint32_t maxMips = 5;
  for (uint32_t i = 0; i < maxMips; i++) {
    uint32_t mipWidth = 128 * std::pow(0.5f, i);
    uint32_t mipHeight = 128 * std::pow(0.5f, i);

    glViewport(0, 0, mipWidth, mipHeight);
    float roughness = (float)i / (float)(maxMips - 1);

    for (uint32_t j = 0; j < 6; j++) {
      RenderPassSpecification spec;
      spec.TargetFramebuffer = Ref<ReflectionBuffer>(this);

      AppRenderEvent e(spec);

      // m_PrefilterShader->UploadMat4("u_View", m_CaptureViews[j]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, m_PrefilterMap,
                             i);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      Renderer3D::DrawCube();
    }
  }
}
} // namespace Lamp