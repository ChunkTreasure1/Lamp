#include "lppch.h"
#include "ImGuiLayer.h"

#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanUtility.h"

#include <imgui/imgui.h>
#include <imnodes.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Lamp
{
	static std::vector<VkCommandBuffer> s_imGuiCommandBuffer;

	namespace Utils
	{
		inline void VulkanCheckResult(VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				LP_CORE_ERROR("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__);
				if (result == VK_ERROR_DEVICE_LOST)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(3s);
				}
				LP_CORE_ASSERT(result == VK_SUCCESS, "");
			}
		}
	}

	ImGuiLayer::ImGuiLayer()
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnDetach()
	{
		vkDeviceWaitIdle(VulkanContext::GetCurrentDevice()->GetHandle());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImNodes::DestroyContext();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(VulkanContext::GetCurrentDevice()->GetHandle(), m_descriptorPool, nullptr);
	}

	void ImGuiLayer::OnAttach()
	{
		//Create imgui layer
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImNodes::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	//Enable keyboard controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	//Enable gamepad controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		//Enable docking

		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		//Enable multiple viewports
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		m_font = io.Fonts->AddFontFromFileTTF("engine/fonts/futura/futura-light.ttf", 18.f);
		io.Fonts->AddFontFromFileTTF("engine/fonts/futura/futura-bold.otf", 18.f);

		//Setup ImGui style
		ImGui::StyleColorsDark();

		//When viewports are enabled tweak WindowRounding 
		//so platform windows can look identical to regular one
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.f;
		}

		style.Colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);

		style.Colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);

		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);

		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);

		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Button] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);


		style.Colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);

		style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabTop] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);

		style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.4f, 0.67f, 1.000f, 0.781f);
		style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);

		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);

		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.GrabMinSize = 7.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 12.0f;
		style.ScrollbarSize = 13.0f;
		style.TabBorderSize = 0.0f;
		style.TabRounding = 0.0f;
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 10.f;

		/////Vulkan Init/////
		auto context = VulkanContext::Get();
		auto swapchain = Application::Get().GetWindow().GetSwapchain();
		auto device = VulkanContext::GetCurrentDevice();

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		LP_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPool));

		Application& app = Application::Get();
		GLFWwindow* pWindow = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForVulkan(pWindow, true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = static_cast<VkInstance>(context->GetInstance());

		initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(context->GetCurrentDevice()->GetPhysicalDevice()->GetHandle());
		initInfo.Device = static_cast<VkDevice>(context->GetCurrentDevice()->GetHandle());
		initInfo.Queue = context->GetCurrentDevice()->GetGraphicsQueue();
		initInfo.DescriptorPool = m_descriptorPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = Utils::VulkanCheckResult;

		ImGui_ImplVulkan_Init(&initInfo, swapchain->GetRenderPass());
		/////////////////////

		VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		device->FlushCommandBuffer(commandBuffer);

		vkDeviceWaitIdle(device->GetHandle());
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		uint32_t framesInFlight = Renderer::Get().GetCapabilities().framesInFlight;
		s_imGuiCommandBuffer.resize(framesInFlight);

		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			s_imGuiCommandBuffer[i] = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();
		}
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGui::Render();

		Ref<Swapchain> swapChain = Application::Get().GetWindow().GetSwapchain();

		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.f} };
		clearValues[1].depthStencil = { 1.f, 0 };

		uint32_t width = swapChain->GetExtent().width;
		uint32_t height = swapChain->GetExtent().height;

		uint32_t commandBufferIndex = swapChain->GetCurrentFrame();

		VkCommandBufferBeginInfo cmdBufferInfo{};
		cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		cmdBufferInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = swapChain->GetDrawCommandBuffer(commandBufferIndex);
		LP_VK_CHECK(vkBeginCommandBuffer(drawCommandBuffer, &cmdBufferInfo));

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.renderPass = swapChain->GetRenderPass();
		beginInfo.renderArea.offset.x = 0;
		beginInfo.renderArea.offset.y = 0;
		beginInfo.renderArea.extent.width = width;
		beginInfo.renderArea.extent.height = height;
		beginInfo.clearValueCount = 2;
		beginInfo.pClearValues = clearValues;
		beginInfo.framebuffer = swapChain->GetCurrentFramebuffer();

		vkCmdBeginRenderPass(drawCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritInfo{};
		inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritInfo.renderPass = swapChain->GetRenderPass();
		inheritInfo.framebuffer = swapChain->GetCurrentFramebuffer();

		VkCommandBufferBeginInfo bufBeginInfo{};
		bufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		bufBeginInfo.pInheritanceInfo = &inheritInfo;

		LP_VK_CHECK(vkBeginCommandBuffer(s_imGuiCommandBuffer[commandBufferIndex], &bufBeginInfo));

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(s_imGuiCommandBuffer[commandBufferIndex], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x;
		scissor.offset.y;
		vkCmdSetScissor(s_imGuiCommandBuffer[commandBufferIndex], 0, 1, &scissor);

		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, s_imGuiCommandBuffer[commandBufferIndex]);

		LP_VK_CHECK(vkEndCommandBuffer(s_imGuiCommandBuffer[commandBufferIndex]));

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(s_imGuiCommandBuffer[commandBufferIndex]);

		vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());
		vkCmdEndRenderPass(drawCommandBuffer);

		LP_VK_CHECK(vkEndCommandBuffer(drawCommandBuffer));

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	ImGuiLayer* ImGuiLayer::Create()
	{
		return new ImGuiLayer();
	}
}