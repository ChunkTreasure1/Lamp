#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/UniformBufferSet.h"
#include "Lamp/Rendering/Buffers/ShaderStorageBufferSet.h"
#include "Lamp/Rendering/Buffers/BufferLayout.h"

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
		TriangleStrip,
		PatchList
	};

	enum class DrawType
	{
		Opaque,
		Transparent,
		Translucency,
		FullscreenQuad,
		Cube,
		Terrain,
		Skybox,
		Quad2D,
		Line2D
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
		Ref<ShaderStorageBufferSet> shaderStorageBufferSets;

		Topology topology;
		DrawType drawType = DrawType::Opaque;
		CullMode cullMode = CullMode::Back;
		bool depthTest = true;
		bool depthWrite = true;
		bool isSwapchain;
		bool useTessellation = false;
		uint32_t tessellationControlPoints = 4;

		std::string debugName;
		BufferLayout vertexLayout;

		std::vector<FramebufferAttachmentInputSpecification> framebufferInputs;
	};

	class RenderPipeline
	{
	public:
	public:
		RenderPipeline(const RenderPipelineSpecification& specification);
		~RenderPipeline();

		void Bind(Ref<CommandBuffer> commandBuffer) const;
		void SetLayout(BufferLayout layout);

		void BindDescriptorSets(Ref<CommandBuffer> commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t startSet = 0) const;
		void BindDescriptorSet(Ref<CommandBuffer> commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const;

		void SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index);
		void SetTexture(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t index);

		void SetPushConstantData(Ref<CommandBuffer> commandBuffer, uint32_t index, const void* data);
		const RenderPipelineSpecification& GetSpecification() const { return m_specification; }

		inline const uint32_t GetDescriptorSetCount() const { return (uint32_t)m_descriptorSets.at(0).size(); }

		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);

	private:
		void CreateDescriptorSets();
		void SetupUniformBuffers();

		void Invalidate();

		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
		VkVertexInputBindingDescription m_bindingDescription;

		uint32_t m_numAttributes;

		VkPipelineLayout m_layout = nullptr;
		VkPipeline m_pipeline = nullptr;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets; //frame->descriptor sets

		RenderPipelineSpecification m_specification;
		
	};
}