#pragma once

namespace Lamp
{
	struct Renderer2DStorage
	{
		/////Rendering//////
		Ref<CommandBuffer> renderCommandBuffer;
		Ref<CommandBuffer> swapchainCommandBuffer;

		Ref<RenderPipeline> quadPipeline;
		Ref<RenderPipeline> linePipeline;
		////////////////////
	};

	class CameraBase;

	class Renderer2D
	{
	public:
		Renderer2D();
		~Renderer2D();

		void Initialize();
		void Shutdown();

		void Begin(const Ref<CameraBase> camera);
		void End();

		void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id = -1);
		void SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color);

	private:
		void SetupRenderPipelines();

		std::unique_ptr<Renderer2DStorage> m_storage;
	};
}