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
		spec.TargetFramebuffer = ShadowBuffer;
		spec.drawType = DrawType::Forward;
		spec.staticUniforms =
		{
			{ 0, { "u_Model", UniformType::RenderData, RenderData::Transform }},
		};

		spec.dynamicUniforms =
		{
			{ 1, { { "u_ViewProjection", UniformType::Mat4, RegisterData(&ViewProjection) }, 2 } }
		};

		spec.renderShader = ShaderLibrary::GetShader("dirShadow");

		ShadowPass = CreateScope<RenderPass>(spec);
	}
}