#pragma once
#include "Lamp/Rendering/Shader/Shader.h"
#include "RendererAPI.h"

#include "Lamp/Core/Application.h"
#include "UniformBuffers.h"
#include "Buffers/UniformBuffer.h"

#include <algorithm>

namespace Lamp
{
	class RenderGraph;
	class ShaderStorageBuffer;
	class CameraBase;
	class Texture2D;
	class Framebuffer;

	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(const Ref<CameraBase> camera);
		static void End();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void SetBufferSize(const glm::vec2& size) { s_pSceneData->bufferSize = size; }

		static const Ref<RenderGraph> GetRenderGraph() { return s_pSceneData->renderGraph; }
		static void SetRenderGraph(Ref<RenderGraph> graph) { s_pSceneData->renderGraph = graph; }

	private:
		static void CreateUniformBuffers();
		static void CreateShaderStorageBuffers();
		static void UpdateBuffers(const Ref<CameraBase> camera);

	private: 
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

			glm::vec2 bufferSize = { 1280, 720 };
			Ref<RenderGraph> renderGraph;

			/////SSAO/////
			uint32_t ssaoKernelSize = 64;
			uint32_t ssaoMaxKernelSize = 256;

			float ssaoRadius = 0.5f;
			float ssaoBias = 0.025f;
			//////////////
		
			/////Uniform buffers//////
			CommonRenderData commonRenderData;
			Ref<UniformBuffer> commonDataBuffer;

			DirectionalLightBuffer directionalLightData;
			Ref<UniformBuffer> directionalLightBuffer;

			DirectionalLightVPs directionalLightVPData;
			Ref<UniformBuffer> directionalLightVPBuffer;
			//////////////////////////

			/////Shader Storage//////
			Ref<ShaderStorageBuffer> visibleLightsStorageBuffer;
			Ref<ShaderStorageBuffer> pointLightStorageBuffer;
			/////////////////////////

			//Internal lists
			std::map<std::string, Ref<Texture2D>> internalTextures;
			std::map<std::string, Ref<Framebuffer>> internalFramebuffers;
			std::vector<Ref<Framebuffer>> useViewportSize;
		};

		static SceneData* s_pSceneData;

		friend class Renderer3D;
		friend class Renderer2D;
		friend class RenderNodeFramebuffer;
		friend class RenderNodePass;
		friend class RenderNodeCompute;
		friend class RenderNodeTexture;
		friend class Level;
	};
}