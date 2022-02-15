#pragma once

#include "Lamp/ImGui/ImGuiLayer.h"

#include <vulkan/vulkan.h>

namespace Lamp
{
	class RenderPipeline;
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		~VulkanImGuiLayer();

		void OnDetach() override;
		void OnAttach() override;

		void Begin() override;
		void End() override;

	private:
		float m_time = 0.f;
		ImFont* m_font;

		VkDescriptorPool m_descriptorPool;
	};
}