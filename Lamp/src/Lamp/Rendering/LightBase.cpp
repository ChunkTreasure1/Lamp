#include "lppch.h"
#include "LightBase.h"

#include "Shader/ShaderLibrary.h"

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

namespace Lamp
{
	DirectionalLight::DirectionalLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.swapchainTarget = false;
		bufferSpec.height = 4096;
		bufferSpec.width = 4096;
		bufferSpec.shadow = true;
		bufferSpec.attachments =
		{
			ImageFormat::RGBA,
			{ ImageFormat::DEPTH32F, TextureFilter::Linear, TextureWrap::Clamp }
		};

		shadowBuffer = Framebuffer::Create(bufferSpec);

		Ref<UniformBufferSet> uniformBufferSet = UniformBufferSet::Create(Renderer::Get().GetCapabilities().framesInFlight);
		uniformBufferSet->Add(&viewProjection, sizeof(glm::mat4), 0, 0);

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = shadowBuffer;
		pipelineSpec.shader = ShaderLibrary::GetShader("directionalShadow");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::Front;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Opaque;
		pipelineSpec.uniformBufferSets = uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		shadowPipeline = RenderPipeline::Create(pipelineSpec);
	}

	PointLight::PointLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.height = 512;
		bufferSpec.width = 512;
	}
}