#include "lppch.h"
#include "LightBase.h"

#include "Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

namespace Lamp
{
	DirectionalLight::DirectionalLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.height = 4096;
		bufferSpec.width = 4096;
		bufferSpec.attachments =
		{
			{ ImageFormat::DEPTH32F, TextureFilter::Linear, TextureWrap::Clamp }
		};

		shadowBuffer = Framebuffer::Create(bufferSpec);
	}

	PointLight::PointLight()
	{
		FramebufferSpecification bufferSpec;
		bufferSpec.height = 512;
		bufferSpec.width = 512;

		shadowBuffer = std::make_shared<PointShadowBuffer>(bufferSpec);
	}
}