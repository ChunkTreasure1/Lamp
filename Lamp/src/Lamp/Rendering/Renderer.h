#pragma once

#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Buffers/UniformBuffer.h"
#include "Lamp/Rendering/RendererDataStructures.h"
#include "Lamp/Rendering/Buffers/RenderBuffer.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include <vulkan/vulkan.h>

#include <algorithm>

namespace Lamp
{
	class RenderComputePipeline;
	class ShaderStorageBufferSet;
	class CameraBase;
	class Texture2D;
	class Framebuffer;
	class Mesh;
	class UniformBufferSet;
	class RenderPipeline;
	class SubMesh;
	class Material;
	class Skybox;
	class Image2D;
	class VulkanRenderer;
	class TextureCube;

	struct GPUMemoryStatistics
	{
		uint64_t allocatedMemory = 0;
		uint64_t freeMemory = 0;
		uint64_t totalGPUMemory = 0;

		uint64_t totalAllocatedMemory = 0;
		uint64_t totalFreedMemory = 0;
	};

	class Renderer
	{
	public:
		struct SceneData;

		static void Initialize();
		static void Shutdown();

		static void Begin(const Ref<CameraBase> camera);
		static void End();

		static void BeginPass(const Ref<RenderPipeline> pipeline);
		static void EndPass();

		static void SwapBuffers();

		static void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id = -1);
		static void SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant = nullptr);
		static void SubmitQuad();

		static void DrawBuffer();

		static const SceneData* GetSceneData() { return s_pSceneData; }
		static Ref<VulkanRenderer> GetRenderer() { return s_renderer; }
		static void GenerateKernel();

		static std::pair<Ref<RenderComputePipeline>, std::function<void()>> CreateLightCullingPipeline(Ref<Image2D> depthImage);

		struct SceneData
		{
			//Data
			const uint32_t maxLights = 1024;
			const uint32_t maxScreenTileBufferAlloc = 2000;	
			const uint32_t screenTileSize = 16;

			uint32_t screenTileCount = 1;
			uint32_t screenGroupX = 1;
			uint32_t screenGroupY = 1;

			float hdrExposure = 1.f;
			float gamma = 2.2f;
			float ambianceMultiplier = 0.5f;

			Ref<Framebuffer> brdfFramebuffer;
			Ref<TextureCube> blackCubeTexture;
			Ref<Texture2D> whiteTexture;

			/////SSAO/////
			uint32_t ssaoMaxKernelSize = 64;
			std::vector<glm::vec4> ssaoNoise;
			Ref<Texture2D> ssaoNoiseTexture;
			//////////////

			/////Uniform buffers//////
			DirectionalLightDataBuffer directionalLightDataBuffer;
			SSAODataBuffer ssaoData;
			CameraDataBuffer cameraData;
			ScreenDataBuffer screenData;
			DirectionalLightVPBuffer directionalLightVPData;
			LightCullingBuffer lightCullingData;
			TerrainDataBuffer terrainData;

			Ref<UniformBufferSet> uniformBufferSet;
			Ref<UniformBuffer> lightCullingBuffer;
			Ref<UniformBuffer> terrainDataBuffer;
			//////////////////////////

			/////Shader Storage//////
			Ref<ShaderStorageBufferSet> shaderStorageBufferSet;
			/////////////////////////
		};

		struct Capabilities
		{
			bool supportAniostopy = false;
			uint32_t maxAniostropy = 1;
			uint32_t framesInFlight = 3;
		};

		struct Statistics
		{
			std::atomic<uint32_t> totalDrawCalls = 0;
			GPUMemoryStatistics memoryStatistics;
		};

		static const Capabilities& GetCapabilities() { return s_capabilities; }
		static const Ref<Texture2D> GetDefaultTexture() { return s_rendererDefaults->defaultTexture; }

		static const Statistics& GetStatistics() { return s_statistics; }

	private:
		static void CreateUniformBuffers(); //TODO: remove

		static void UpdateBuffers(const Ref<CameraBase> camera);
		static void UpdatePassBuffers(const Ref<RenderPipeline> pipeline);
		static void DrawDirectionalShadows();
		static void GenerateBRDF();
		static void SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& buffer);

	private:
		struct RendererDefaults
		{
			Ref<Texture2D> defaultTexture;
		};

		static SceneData* s_pSceneData;
		static Capabilities s_capabilities;
		static Statistics s_statistics;
		
		static Ref<VulkanRenderer> s_renderer;
		static Scope<RendererDefaults> s_rendererDefaults;
		static Scope<Skybox> s_skybox;

		static RenderBuffer s_firstRenderBuffer;
		static RenderBuffer s_secondRenderBuffer;

		static RenderBuffer* s_submitBufferPointer;
		static RenderBuffer* s_renderBufferPointer;

		friend class VulkanPhysicalDevice;

		friend class Renderer3D;
		friend class Renderer2D;
		friend class Skybox;
		friend class RenderNodeFramebuffer;
		friend class RenderNodePass;
		friend class RenderNodeCompute;
		friend class RenderNodeTexture;
		friend class Level;
	};
}