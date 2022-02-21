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
	class TextureCube;
	class RenderComputePipeline;
	class Image2D;
	class UniformBuffer;
	class ShaderStorageBuffer;
	class ShaderStorageBufferSet;
	class Terrain;
	class SubMesh;
	class CameraBase;
	class Material;
	class MaterialLibrary;

	struct RendererStatistics
	{
		std::atomic<uint32_t> totalDrawCalls = 0;
		std::atomic<uint32_t> culledDrawCalls = 0;
		GPUMemoryStatistics memoryStatistics;
	};

	struct RendererCapabilities
	{
		bool supportAniostopy = false;
		uint32_t maxAniostropy = 1;
		uint32_t framesInFlight = 3;
		uint32_t maxShaderTexturesArray = 1;
	};

	struct RendererDefaults
	{
		Ref<Framebuffer> brdfFramebuffer;
		Ref<TextureCube> blackCubeTexture;
		Ref<Texture2D> whiteTexture;
	};

	struct VulkanRendererStorage
	{
		/////Rendering/////
		Ref<CommandBuffer> swapchainCommandBuffer;
		Ref<CommandBuffer> renderCommandBuffer;

		Ref<RenderPipeline> currentRenderPipeline;
		Ref<CameraBase> camera;

		Ref<SubMesh> quadMesh;

		std::vector<VkDescriptorSet> pipelineDescriptorSets;
		std::vector<VkDescriptorSet> currentMeshDescriptorSets;

		LightCullingRendererData lightCullingRendererData;
		///////////////////

		/////Shader storage//////
		DirectionalLightDataBuffer directionalLightDataBuffer;
		CameraDataBuffer cameraData;
		ScreenDataBuffer screenData;
		DirectionalLightVPBuffer directionalLightVPData;
		LightCullingBuffer lightCullingData;
		TerrainDataBuffer terrainData;

		Ref<UniformBufferSet> uniformBufferSet;
		Ref<UniformBuffer> lightCullingBuffer;
		Ref<UniformBuffer> terrainDataBuffer;

		Ref<ShaderStorageBufferSet> shaderStorageBufferSet;
		/////////////////////////
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Initialize();
		void Shutdown();

		void Begin(const Ref<CameraBase> camera);
		void End();
		void SwapRenderBuffers();

		void BeginPass(Ref<RenderPipeline> pipeline);
		void EndPass();

		const GPUMemoryStatistics& GetMemoryUsage() const;
		const RendererStatistics& GetStatistics() const;
		const RendererCapabilities& GetCapabilities() const;
		const VulkanRendererStorage& GetStorage() const;
		const RendererDefaults& GetDefaults() const;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id = -1);
		void SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant = nullptr);

		void DrawMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);
		void DrawMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant = nullptr);
		void DrawQuad();

		void DispatchRenderCommands(RenderBuffer& buffer);
		void DispatchRenderCommands();

		VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo);
		std::pair<Ref<RenderComputePipeline>, std::function<void()>> CreateLightCullingPipeline(Ref<Image2D> depthImage);

		static Renderer& Get();
		static Ref<Renderer> Create();

	private:
		void AllocateDescriptorsForMaterialRendering(Ref<Material> material);
		void AllocateDescriptorsForQuadRendering(); // TODO: should be moved into singular function
		void AllocatePerPassDescriptors();

		void PreAllocateRenderBuffers();
		void ClearRenderBuffers();

		void AllocateDescriptorPools();
		void DestroyDescriptorPools();

		void CreateRendererStorage();
		void DestroyRendererStorage();
		void CreateUniformBuffers();
		void UpdateUniformBuffers();
		void UpdatePerPassUniformBuffers(const Ref<RenderPipeline> pipeline);
		void GenerateBRDF(Ref<Framebuffer>& outFramebuffer);

		void PrepareForRender();
		void FrustumCull();
		void SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& renderBuffer);
		void DrawDirectionalShadow();

		friend class VulkanPhysicalDevice;

		static Renderer* s_instance;
		static RendererCapabilities s_capabilities;

		/////Storage/////
		Scope<VulkanRendererStorage> m_rendererStorage;
		Scope<RendererDefaults> m_rendererDefaults;
		Scope<MaterialLibrary> m_materialLibrary;
		/////////////////
		
		/////Render buffer/////
		RenderBuffer m_firstRenderBuffer;
		RenderBuffer m_secondRenderBuffer;
		RenderBuffer m_finalRenderBuffer;

		RenderBuffer* m_submitBufferPointer = &m_firstRenderBuffer;
		RenderBuffer* m_renderBufferPointer = &m_secondRenderBuffer;
		///////////////////////

		RendererStatistics m_statistics;

		std::vector<VkDescriptorPool> m_descriptorPools;
		std::atomic_bool m_renderingFinished = true;
	};
}