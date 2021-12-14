#pragma once

#include "Lamp/Rendering/Buffers/VertexArray.h"

#include <filesystem>

namespace Lamp
{
	class Shader;
	class TextureCube;
	class TextureHDR;
	class Mesh;
	class Image2D;
	class Skybox
	{
	public:
		Skybox(const std::filesystem::path& path);
		~Skybox();

		void Draw();

	public:
		static Ref<Skybox> Create(const std::filesystem::path& path) { return CreateRef<Skybox>(path); }

	private:
		void GenerateBRDFLUT();
		void GenerateIrradianceCube();

		//Shaders
		Ref<Shader> m_eqCubeShader;
		Ref<Shader> m_convolutionShader;
		Ref<Shader> m_prefilterShader;
		Ref<Shader> m_skyboxShader;

		//Textures
		Ref<TextureCube> m_cubeMap;
		Ref<TextureCube> m_irradianceMap;
		Ref<TextureCube> m_prefilterMap;
		Ref<TextureHDR> m_hdrTexture;

		Ref<Framebuffer> m_brdfFramebuffer;

		//Data
		glm::mat4 m_captureProjection;
		std::array<glm::mat4, 6> m_captureViews;
		Ref<Mesh> m_cubeMesh;
		Ref<Framebuffer> m_framebuffer;
	};
}