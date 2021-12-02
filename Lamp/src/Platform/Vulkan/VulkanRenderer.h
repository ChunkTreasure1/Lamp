#pragma once

#include "Lamp/Rendering/RendererNew.h"

namespace Lamp
{
	class VulkanRenderer : public RendererNew
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Begin(const Ref<CameraBase> camera) override;
		void End() override;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);
	};
}