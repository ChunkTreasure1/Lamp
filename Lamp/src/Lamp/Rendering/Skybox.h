#pragma once

#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include "Lamp/AssetSystem/Asset.h"

#include "Platform/Vulkan/VulkanShader.h"

#include <filesystem>

namespace Lamp
{
	class TextureCube;
	class SubMesh;
	class Image2D;
	class RenderPipeline;

	class Skybox : public Asset
	{
	public:
		Skybox(const std::filesystem::path& path);
		~Skybox();

		inline Ref<TextureCube> GetIrradiance() const { return m_irradianceMap; }
		inline Ref<TextureCube> GetFilteredEnvironment() const { return m_filteredEnvironment; }
		inline Ref<Image2D> GetBRDF() const { return m_brdfFramebuffer->GetColorAttachment(0); }
		inline Ref<RenderPipeline> GetPipeline() const { return m_pipeline; }

		static AssetType GetStaticType() { return AssetType::EnvironmentMap; }
		AssetType GetType() override { return GetStaticType(); }

		void Draw();

		static Ref<Skybox> Create(const std::filesystem::path& path) { return CreateRef<Skybox>(path); }

	private:
		void SetupDescriptors();

		Ref<TextureCube> m_irradianceMap;
		Ref<TextureCube> m_filteredEnvironment;
		Ref<SubMesh> m_cubeMesh;

		Ref<Framebuffer> m_brdfFramebuffer;
		Ref<RenderPipeline> m_pipeline;

		VulkanShader::ShaderMaterialDescriptorSet m_descriptorSet;
	};
}