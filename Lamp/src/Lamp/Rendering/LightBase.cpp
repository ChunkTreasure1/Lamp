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
		spec.targetFramebuffer = shadowBuffer;
		spec.drawType = DrawType::Forward;

		spec.uniforms =
		{
			{ "u_Model", UniformType::RenderData, RenderData::Transform, 0, 0, 1},
			{ RegisterData(&viewProjection), "u_ViewProjection", UniformType::Mat4 }
		};

		spec.renderShader = ShaderLibrary::GetShader("dirShadow");

		shadowPass = CreateScope<RenderPass>(spec);
	}
}