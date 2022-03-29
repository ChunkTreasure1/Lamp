#include "lppch.h"
#include "RenderCommand.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	Renderer* RenderCommand::s_renderer = nullptr;
	Renderer2D* RenderCommand::s_renderer2D = nullptr;

	void RenderCommand::Initialize(Renderer* renderer, Renderer2D* renderer2D)
	{
		LP_CORE_ASSERT(renderer, "Renderer was nullptr! Unable to initialize RenderCommand!");
		LP_CORE_ASSERT(renderer2D, "Renderer was nullptr! Unable to initialize RenderCommand!");

		s_renderer = renderer;
		s_renderer2D = renderer2D;
	}

	void RenderCommand::Shutdown()
	{
		LP_CORE_ASSERT(s_renderer, "Renderer was already nullptr! Something has gone wrong!");
		LP_CORE_ASSERT(s_renderer2D, "Renderer was already nullptr! Something has gone wrong!");
		s_renderer = nullptr;
		s_renderer2D = nullptr;
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
		s_renderer2D->SwapRenderBuffers();
	}

	void RenderCommand::ClearFrame()
	{
		s_renderer->ClearFrame();
	}

	void RenderCommand::BeginPass(const Ref<RenderPipeline> pipeline)
	{
		s_renderer->BeginPass(pipeline);
	}

	void RenderCommand::EndPass()
	{
		s_renderer->EndPass();
	}

	void RenderCommand::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, size_t id)
	{
		s_renderer->SubmitMesh(transform, mesh, material, id);
	}

	void RenderCommand::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		s_renderer->SubmitMesh(mesh, material, descriptorSets, pushConstant);
	}

	void RenderCommand::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id)
	{
		s_renderer2D->SubmitQuad(transform, color, texture, id);
	}

	void RenderCommand::SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color)
	{
		s_renderer2D->SubmitLine(pointOne, pointTwo, color);
	}

	void RenderCommand::DrawMeshDirect(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, size_t id)
	{
		s_renderer->DrawMesh(transform, mesh, material, id);
	}

	void RenderCommand::DispatchRenderCommands()
	{
		s_renderer->DispatchRenderCommands();
	}
}