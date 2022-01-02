#pragma once

#include "Lamp/Rendering/Buffers/VertexArray.h"

#include <filesystem>

namespace Lamp
{
	class Shader;
	class TextureCube;
	class Texture2D;
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
		void GenerateEquirectangularCube();

		const glm::mat4 m_perspective = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f);

		const std::array<glm::mat4, 6> m_viewMatrices =
		{
			//Positive X
			glm::rotate(glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)),

			//Negative X
			glm::rotate(glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)),

			//Positive Y
			glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),

			//Negative Y
			glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)),

			//Positive Z
			glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)),

			//Negative Z
			glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
		};

		//Textures
		Ref<TextureCube> m_cubeMap;
		Ref<TextureCube> m_irradianceMap;
		Ref<TextureCube> m_prefilterMap;

		Ref<Texture2D> m_hdrTexture;
		Ref<Framebuffer> m_brdfFramebuffer;
	};
}