#pragma once

#include "Lamp/Rendering/RendererDataStructures.h"
#include "Lamp/Rendering/Buffers/RenderBuffer.h"

#include "Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan_core.h>
#include <atomic>

namespace Lamp
{
	class RenderPipeline;
	class Shader;
	class CommandBuffer;
	class UniformBufferSet;
	class Mesh;
	class Texture2D;
	class RenderComputePipeline;
	class Image2D;
	class UniformBuffer;
	class ShaderStorageBuffer;
	class ShaderStorageBufferSet;
	class Terrain;
	class SubMesh;
	class CameraBase;
	class Material;

	struct VulkanRendererStorage
	{
		Ref<CommandBuffer> swapchainCommandBuffer;
		Ref<CommandBuffer> renderCommandBuffer;

		Ref<RenderPipeline> currentRenderPipeline;
		Ref<CameraBase> camera;

		Ref<SubMesh> quadMesh;
		Ref<SubMesh> cubeMesh;

		std::vector<VkDescriptorSet> pipelineDescriptorSets;
		std::vector<VkDescriptorSet> currentMeshDescriptorSets;

		std::vector<VulkanShader::ShaderMaterialDescriptorSet> shaderDescriptorSets;
	};

	class VulkanRenderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Initialize();
		void Shutdown();
		void Begin(const Ref<CameraBase> camera);
		void End();

		void BeginPass(Ref<RenderPipeline> pipeline);
		void EndPass();

		const GPUMemoryStatistics& GetMemoryUsage() const;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);
		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets);
		void SubmitQuad();

		void DrawSkybox();
		void DrawTerrain();
		void DrawBuffer(RenderBuffer& buffer);

		inline VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }
		
		std::pair<Ref<RenderComputePipeline>, std::function<void()>> CreateLightCullingPipeline(Ref<UniformBuffer> cameraDataBuffer, Ref<UniformBuffer> lightCullingBuffer, Ref<ShaderStorageBufferSet> shaderStorageSet, Ref<Image2D> depthImage);

		static Ref<VulkanRenderer> Create();

	private:
		void SetupDescriptorsForMaterialRendering(Ref<Material> material);
		void SetupDescriptorsForQuadRendering(); // TODO: should be moved into singular function
		void SetupDescriptorsForSkyboxRendering();
		void SetupDescriptorsForTerrainRendering();

		void FreePipelineDescriptors();
		void UpdatePerPassDescriptors();

		void CreateBaseGeometry();
		void CreateSkyboxPipeline(Ref<Framebuffer> framebuffer);
		void CreateTerrainPipeline(Ref<Framebuffer> framebuffer);

		Scope<VulkanRendererStorage> m_rendererStorage;

		Ref<RenderPipeline> m_skyboxPipeline;
		Ref<RenderPipeline> m_terrainPipeline;

		VkDescriptorPool m_descriptorPool;

		std::atomic_bool m_renderingFinished = true;
	};
}