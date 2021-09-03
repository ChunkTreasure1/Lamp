#pragma once

#include <functional>
#include <Lamp/Rendering/Vertices/FrameBuffer.h>
#include <Lamp/Core/Core.h>

namespace Sandbox3D
{
	class BufferWindow
	{
	public:
		BufferWindow(Ref<Lamp::Framebuffer> framebuffer, const std::string& name)
			: m_FrameBuffer(framebuffer), m_Name(name)
		{
			for (auto& att : m_FrameBuffer->GetSpecification().Attachments.Attachments)
			{
				if ((uint32_t)att.TextureFormat <= 5)
				{
					m_TextureId = m_FrameBuffer->GetColorAttachmentID(0);

					break;
				}
			}

			if (m_TextureId == -1)
			{
				m_TextureId = m_FrameBuffer->GetDepthAttachmentID();
			}
		}

		void Update()
		{
			if (!m_IsOpen)
			{
				return;
			}

			ImGui::Begin(m_Name.c_str(), &m_IsOpen);

			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			ImGui::Image((void*)(uint64_t)m_TextureId, panelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			ImGui::End();
		}

		const std::string& GetLabel() { return m_Name; }
		bool& GetIsOpen() { return m_IsOpen; }

	private:
		Ref<Lamp::Framebuffer> m_FrameBuffer;
		uint32_t m_TextureId = -1;
		std::string m_Name;
		bool m_IsOpen = false;
	};
}