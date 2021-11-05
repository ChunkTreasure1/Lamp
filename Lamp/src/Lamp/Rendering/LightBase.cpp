#include "lppch.h"
#include "LightBase.h"

#include "RenderPass.h"
#include "Shader/ShaderLibrary.h"

namespace Lamp
{
	DirectionalLight::DirectionalLight()
	{
		RenderPassSpecification spec;
		spec.clearType = ClearType::ColorDepth;
		spec.cullFace = CullFace::Front;
		spec.TargetFramebuffer = shadowBuffer;
		spec.drawType = DrawType::Forward;

		spec.uniforms =
		{
			{ 0, {{ "u_Model", UniformType::RenderData, RenderData::Transform }, 1}},
			{ 2, { { RegisterData(&viewProjection), "u_ViewProjection", UniformType::Mat4 }, 3 } }
		};

		spec.renderShader = ShaderLibrary::GetShader("dirShadow");

		shadowPass = CreateScope<RenderPass>(spec);
	}
}