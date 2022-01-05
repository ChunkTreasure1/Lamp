#pragma once

#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include <filesystem>

namespace Lamp
{
	class Shader;
	class TextureCube;
	class Texture2D;
	class Mesh;
	class Image2D;
	class RenderPipeline;

	class Skybox
	{
	public:
		Skybox(const std::filesystem::path& path);
		~Skybox();

		inline Ref<TextureCube> GetIrradiance() const { return m_irradianceMap; }
		inline Ref<TextureCube> GetFilteredEnvironment() const { return m_filteredEnvironment; }
		inline Ref<Image2D> GetBRDF() const { return m_brdfFramebuffer->GetColorAttachment(0); }

	public:
		static Ref<Skybox> Create(const std::filesystem::path& path) { return CreateRef<Skybox>(path); }

	private:
		Ref<TextureCube> m_irradianceMap;
		Ref<TextureCube> m_filteredEnvironment;

		Ref<Framebuffer> m_brdfFramebuffer;
	};
}