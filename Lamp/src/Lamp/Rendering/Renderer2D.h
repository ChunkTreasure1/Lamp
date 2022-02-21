#pragma once

#include "Lamp/Rendering/Buffers/RenderBuffer.h"
#include "Lamp/Rendering/Vertex.h"

#include "Lamp/Rendering/RenderPipeline.h"

#include <vulkan/vulkan.h>

namespace Lamp
{
	class CameraBase;
	class Texture2D;

	struct Renderer2DStorage
	{
		/////Rendering//////
		Ref<CommandBuffer> renderCommandBuffer;
		Ref<CommandBuffer> swapchainCommandBuffer;

		Ref<RenderPipeline> quadPipeline;
		Ref<RenderPipeline> linePipeline;

		VkDescriptorSet currentQuadDescriptorSet;
		VkDescriptorSet currentLineDescriptorSet;
		////////////////////

		/////Values/////
		static const uint32_t maxQuads = 20000;
		static const uint32_t maxVertices = maxQuads * 4;
		static const uint32_t maxIndices = maxQuads * 6;

		static const uint32_t maxLines = 10000;
		static const uint32_t maxLineVerts = maxLines * 2;
		static const uint32_t maxLineIndices = maxLines * 2;
		////////////////

		/////Quad Storage/////
		Ref<VertexBuffer> quadVertexBuffer;
		Ref<IndexBuffer> quadIndexBuffer;

		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;
		glm::vec4 quadVertexPositions[4];

		Ref<Texture2D> whiteTexture;
		Ref<Texture2D>* textureSlots;

		uint32_t textureSlotIndex = 1;
		uint32_t quadIndexCount = 0;
		//////////////////////

		/////Line Storage/////
		
		//////////////////////
	};

	class Renderer2D
	{
	public:
		Renderer2D();
		~Renderer2D() = default;

		void Initialize();
		void Shutdown();

		void Begin(const Ref<CameraBase> camera);
		void End();

		void SwapRenderBuffers();
		void DispatchRenderCommands(DrawType type);

		void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id = -1);
		void SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color);

		void DrawQuad(const RenderCommandData& cmd);

		Ref<RenderPipeline> CreateQuadPipeline(Ref<Framebuffer> framebuffer);
		Ref<RenderPipeline> CreateLinePipeline(Ref<Framebuffer> framebuffer);

		static Renderer2D& Get() { return *s_instance; }
		
	private:
		void CreateStorage();
		void AllocateAndUpdateDescriptors();

		void ResetBatchData();
		void StartNewBatch();
		void Flush();

		/////Render buffer/////
		RenderBuffer m_firstRenderBuffer;
		RenderBuffer m_secondRenderBuffer;

		RenderBuffer* m_submitBufferPointer = &m_firstRenderBuffer;
		RenderBuffer* m_renderBufferPointer = &m_secondRenderBuffer;
		///////////////////////

		static Renderer2D* s_instance;

		std::unique_ptr<Renderer2DStorage> m_storage;
	};
}