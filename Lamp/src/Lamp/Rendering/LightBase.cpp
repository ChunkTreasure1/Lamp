#include "lppch.h"
#include "LightBase.h"

#include "RenderPass.h"
#include "Shader/ShaderLibrary.h"

namespace Lamp
{
	DirectionalLight::DirectionalLight()
	{
		const float size = 10.f;
		glm::mat4 viewProjection = glm::ortho(-size * Direction.x, size * Direction.x, -size * Direction.z, size * Direction.z, 0.1f, 1000.f) * glm::lookAt(Direction, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		RenderPassSpecification spec;
		spec.clearType = ClearType::ColorDepth;
		spec.cullFace = CullFace::Front;
		spec.TargetFramebuffer = ShadowBuffer;
		spec.drawType = DrawType::Forward;
		spec.staticUniforms =
		{
			{ 0, { "u_Model", UniformType::RenderData, RenderData::Transform }},
			{ 1, { "u_ViewProjection", UniformType::Mat4, viewProjection }}
		};

		spec.renderShader = ShaderLibrary::GetShader("dirShadow");

		ShadowPass = CreateScope<RenderPass>(spec);
	}
}