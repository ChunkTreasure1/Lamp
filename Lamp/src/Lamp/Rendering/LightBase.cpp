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
			{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
		};

		shadowBuffer = Framebuffer::Create(bufferSpec);

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

		spec.uniforms =
		{
			{ RegisterData(&farPlane), "u_FarPlane", UniformType::Float },
			{ RegisterData(&const_cast<glm::vec3&>(shadowBuffer->GetPosition())), "u_LightPosition", UniformType::Float3 },
			{ "u_Model", UniformType::RenderData, RenderData::Transform },
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[0])), "u_Transforms[0]", UniformType::Mat4},
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[1])), "u_Transforms[1]", UniformType::Mat4},
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[2])), "u_Transforms[2]", UniformType::Mat4},
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[3])), "u_Transforms[3]", UniformType::Mat4},
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[4])), "u_Transforms[4]", UniformType::Mat4},
			{ RegisterData(&const_cast<glm::mat4&>(shadowBuffer->GetTransforms()[5])), "u_Transforms[5]", UniformType::Mat4}
		};

		spec.renderShader = ShaderLibrary::GetShader("pointShadow");

		shadowPass = CreateScope<RenderPass>(spec);
	}
}