#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/UniformBufferSet.h"

namespace Lamp
{
	class Shader;
	class CommandBuffer;
	class Image2D;
	class Texture2D;
	class TextureCube;

	enum class Topology
	{
		TriangleList,
		LineList,
		TriangleStrip
	};

	enum class DrawType
	{
		Buffer,
		Quad,
		Cube
	};

	enum class CullMode
	{
		Front,
		Back,
		FrontAndBack,
		None
	};

	struct FramebufferAttachmentInputSpecification
	{
		Ref<Image2D> attachment;
		uint32_t set;
		uint32_t binding;
	};

	struct TextureInputSpecification
	{
		Ref<Texture2D> texture;
		uint32_t set;
		uint32_t binding;
	};

	struct TextureCubeInputSpecification
	{
		Ref<TextureCube> texture;
		uint32_t set;
		uint32_t binding;
	};

	struct RenderPipelineSpecification
	{
		Ref<Framebuffer> framebuffer;
		Ref<Shader> shader;
		Ref<UniformBufferSet> uniformBufferSets;

		Topology topology;
		DrawType drawType = DrawType::Buffer;
		CullMode cullMode = CullMode::Back;
		bool drawSkybox = false;

		BufferLayout vertexLayout;

		std::vector<FramebufferAttachmentInputSpecification> framebufferInputs;
		std::vector<TextureInputSpecification> textureInputs;
		std::vector<TextureCubeInputSpecification> textureCubeInputs;

		glm::vec2 size;
		bool isSwapchain;
	};

	class RenderPipeline
	{
	public:
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;
		virtual void SetLayout(BufferLayout layout) = 0;
		virtual const RenderPipelineSpecification& GetSpecification() const = 0;
		
		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);
	};

	class RenderComputePipeline
	{
	public:
		virtual void Begin(Ref<CommandBuffer> commandBuffer = nullptr) = 0;
		virtual void End() = 0;

		virtual Ref<Shader> GetShader() = 0;
		static Ref<RenderComputePipeline> Create(Ref<Shader> computeShader);
	};
}