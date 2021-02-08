#include "lppch.h"
#include "OpenGLTexture.h"

#include "Lamp/Rendering/Texture2D/TextureCache.h"

namespace Lamp
{
OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
    : m_Width(width), m_Height(height)
{
    m_InternalFormat = GL_RGBA8, m_DataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

    //Set texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Set filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
{
    m_Path = path;

    auto tex = TextureCache::GetTexture(path);
    m_RendererID = std::get<0>(tex);
    m_Width = std::get<1>(tex);
    m_Height = std::get<2>(tex);
    m_InternalFormat = std::get<3>(tex);
    m_DataFormat = std::get<4>(tex);
}

void OpenGLTexture2D::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_RendererID);
}

void OpenGLTexture2D::SetData(void* data, uint32_t size)
{
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}
}