#pragma once
#include "Lamp/Rendering/Shader/Shader.h"
#include "RendererAPI.h"

#include "Lamp/Core/Application.h"
#include "UniformBuffers.h"
#include "Buffers/UniformBuffer.h"

#include "Lamp/Rendering/RendererDataStructures.h"

#include <algorithm>

namespace Lamp
{
	class RendererNew;
	class ShaderStorageBuffer;
	class CameraBase;
	class Texture2D;
	class Framebuffer;
	class Mesh;
	class UniformBufferSet;
	class RenderPipeline;

	class Renderer
	{
	public:
		struct SceneData;

		static void Initialize();
		static void Shutdown();

		static void Begin(const Ref<CameraBase> camera);
		static void End();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void SetBufferSize(const glm::vec2& size) { s_pSceneData->bufferSize = size; }

		static const SceneData* GetSceneData() { return s_pSceneData; }
		static Ref<RendererNew> GetRenderer() { return s_renderer; }
		static void GenerateKernel();

		static void SetupBuffers(); //TODO: remove
		static void SwapchainBegin(); //TODO: remove
		static void SwapchainEnd(); //TODO: remove
		static void GeometryPass(); //TODO: remove
		static Ref<Framebuffer> GetFramebuffer(); //TODO: remove

		struct SceneData
		{
			//Data
			const uint32_t maxLights = 1024;
			const uint32_t maxScreenTileBufferAlloc = 2000;
			const uint32_t screenTileSize = 16;

			uint32_t screenTileCount = 1;
			uint32_t screenGroupX = 1;
			uint32_t screenGroupY = 1;
			uint32_t pointLightCount = 0;

			float hdrExposure = 3.f;
			float gamma = 2.2f;
			float ambianceMultiplier = 0.3f;

			glm::vec2 bufferSize = { 1280, 720 };

			/////SSAO/////
			uint32_t ssaoMaxKernelSize = 64;
			std::vector<glm::vec3> ssaoNoise;
			Ref<Texture2D> ssaoNoiseTexture;

			float aspectRatio = 16.f / 9.f;
			float tanHalfFOV = 0.f;
			//////////////

			/////Uniform buffers//////
			CommonRenderData commonRenderData;
			Ref<UniformBuffer> commonDataBuffer;

			DirectionalLightBuffer directionalLightData;
			Ref<UniformBuffer> directionalLightBuffer;

			DirectionalLightVPs directionalLightVPData;
			Ref<UniformBuffer> directionalLightVPBuffer;

			SSAOData ssaoData;
			Ref<UniformBuffer> ssaoBuffer;
			//////////////////////////

			/////Shader Storage//////
			Ref<ShaderStorageBuffer> visibleLightsStorageBuffer;
			Ref<ShaderStorageBuffer> pointLightStorageBuffer;
			/////////////////////////

			//Internal lists
			std::map<std::string, Ref<Texture2D>> internalTextures;
			std::map<std::string, Ref<Framebuffer>> internalFramebuffers;
			std::vector<Ref<Framebuffer>> useViewportSize;
			
			///////////////////TESTING/////////////////
			Ref<Mesh> teddy;
			Ref<Shader> mainShader;
			
			Ref<RenderPipeline> swapchainPipeline;
			Ref<RenderPipeline> geometryPipeline;

			CameraDataBuffer cameraBuffer;
			DirectionalLightDataTest directionalLightBufferTest;
			Ref<UniformBufferSet> uniformBufferSet;
		};

		struct Capabilities
		{
			bool supportAniostopy = false;
			uint32_t maxAniostropy = 1;

			uint32_t framesInFlight = 3;
		};

		struct Statistics
		{
			uint32_t totalDrawCalls = 0;
		
			uint64_t memoryUsage = 0;
		};

		static const Capabilities& GetCapabilities() { return s_capabilities; }
		static const Ref<Texture2D> GetDefaultTexture() { return s_rendererDefaults->defaultTexture; }

		static const Statistics& GetStatistics() { return s_statistics; }

	private:
		static void CreateUniformBuffers();
		static void CreateShaderStorageBuffers();
		static void UpdateBuffers(const Ref<CameraBase> camera);

	private:
		struct RendererDefaults
		{
			Ref<Texture2D> defaultTexture;
		};

		static SceneData* s_pSceneData;
		static Capabilities s_capabilities;
		static Statistics s_statistics;
		
		static Ref<RendererNew> s_renderer;
		static Scope<RendererDefaults> s_rendererDefaults;


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