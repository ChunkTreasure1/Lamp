#pragma once

#include "Lamp/Rendering/RendererNew.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class RenderPipeline;
	class Shader;
	class CommandBuffer;
	class UniformBufferSet;
	class Mesh;
	class Texture2D;

	struct MeshDataBuffer
	{
		alignas(16) glm::mat4 model;
	};

	struct CameraDataBuffer
	{
		alignas(16) glm::vec4 position;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	struct DirectionalLightDataTest
	{
		alignas(16) glm::vec4 direction;
		alignas(16) glm::vec4 colorIntensity;
	};

	struct TempRendererStorage
	{
		Ref<RenderPipeline> mainPipeline;
		Ref<Shader> mainShader;
		Ref<CommandBuffer> commandBuffer;

		MeshDataBuffer meshBuffer;
		CameraDataBuffer cameraBuffer;
		DirectionalLightDataTest directionalLightBuffer;

		Ref<UniformBufferSet> uniformBufferSet;


		Ref<Mesh> teddy;
	};

	class VulkanRenderer : public RendererNew
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Initialize() override;
		void Shutdown() override;
		void Begin(const Ref<CameraBase> camera) override;
		void End() override;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);

		inline VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

	private:
		void SetupBuffers();

		Scope<TempRendererStorage> m_rendererStorage;
		VkDescriptorPool m_descriptorPool;
	};
}