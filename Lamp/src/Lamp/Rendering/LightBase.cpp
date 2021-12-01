#include "lppch.h"
#include "LightBase.h"

#include "RenderPass.h"
#include "Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

namespace Lamp
{
	DirectionalLight::DirectionalLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.Height = 4096;
		bufferSpec.Width = 4096;
		bufferSpec.Attachments =
		{
			{ FramebufferTextureFormat::DEPTH32F, FramebufferTextureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
		};

		shadowBuffer = Framebuffer::Create(bufferSpec);

		RenderPassSpecification spec;
		spec.clearType = ClearType::ColorDepth;
		spec.cullFace = CullFace::Front;
		spec.targetFramebuffer = shadowBuffer;
		spec.drawType = DrawType::Forward;

		spec.renderShader = ShaderLibrary::GetShader("dirShadow");

		shadowPass = CreateScope<RenderPass>(spec);
	}

	PointLight::PointLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.Height = 512;
		bufferSpec.Width = 512;

		shadowBuffer = std::make_shared<PointShadowBuffer>(bufferSpec);

		RenderPassSpecification spec;
		spec.clearType = ClearType::Depth;
		spec.cullFace = CullFace::Back;
		spec.targetFramebuffer = std::dynamic_pointer_cast<Framebuffer>(shadowBuffer);
		spec.drawType = DrawType::Forward;

		spec.renderShader = ShaderLibrary::GetShader("pointShadow");

		shadowPass = CreateScope<RenderPass>(spec);
	}
}