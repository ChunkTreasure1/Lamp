#include "lppch.h"
#include "RenderCommand.h"

#include "Platform/Vulkan/VulkanRenderer.h"

namespace Lamp
{
	Renderer* RenderCommand::s_renderer = nullptr;

	void RenderCommand::Initialize(Renderer* renderer)
	{
		LP_CORE_ASSERT(renderer, "Renderer was nullptr! Unable to initialize RenderCommand!");

		s_renderer = renderer;
	}

	void RenderCommand::Shutdown()
	{
		LP_CORE_ASSERT(s_renderer, "Renderer was already nullptr! Something has gone wrong!");
		s_renderer = nullptr;
	}

	void RenderCommand::Begin(const Ref<CameraBase> camera)
	{
		s_renderer->Begin(camera);
	}

	void RenderCommand::End()
	{
		s_renderer->End();
	}

	void RenderCommand::SwapRenderBuffers()
	{
		s_renderer->SwapRenderBuffers();
	}

	void RenderCommand::BeginPass(const Ref<RenderPipeline> pipeline)
	{
		s_renderer->BeginPass(pipeline);
	}

	void RenderCommand::EndPass()
	{
		s_renderer->EndPass();
	}

	void RenderCommand::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		s_renderer->SubmitMesh(transform, mesh, material, id);
	}

	void RenderCommand::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		s_renderer->SubmitMesh(mesh, material, descriptorSets, pushConstant);
	}

	void RenderCommand::DispatchRenderCommands()
	{
		s_renderer->DispatchRenderCommands();
	}
}