#include "kablunkpch.h"

#include <vulkan/vulkan.h>

#include "kablunk/renderer/backend/vulkan/vulkan_imgui_layer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"

#include <examples/imgui_impl_vulkan_with_textures.h>
#include <backends/imgui_impl_glfw.h>


#include <ImGuizmo.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

static std::vector<VkCommandBuffer> s_imgui_command_buffers;

vulkan_imgui_layer::vulkan_imgui_layer()
{

}

vulkan_imgui_layer::vulkan_imgui_layer(const std::string& name)
{
	m_DebugName = name;
}

vulkan_imgui_layer::~vulkan_imgui_layer()
{

}

void vulkan_imgui_layer::OnAttach()
{
    KB_PROFILE_SCOPE;
	KB_CORE_INFO("VulkanImGuiLayer OnAttach called!");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// #TODO build font library to load fonts instead of current way
	// #TODO font scaling
	//io.Fonts->AddFontFromFileTTF("resources/fonts/poppins/Poppins-Bold.ttf", 28.0f);
    //io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/poppins/Poppins-Medium.ttf", 28.0f);
    //io.Fonts->AddFontFromFileTTF("resources/fonts/inter/Inter-Bold.ttf", 28.0f);
    //io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/inter/Inter-Light.ttf", 28.0f);
    io.Fonts->AddFontFromFileTTF("resources/fonts/spiegel/Spiegel_TT_Bold.ttf", 28.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/spiegel/Spiegel_TT_Regular.ttf", 28.0f);
	//io.Fonts->Build();

	//ImGui::StyleColorsDark();
	SetDarkTheme();

	// tweak windowround when viewports are enabled so they look like Windows platform windows
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

	vulkan_imgui_layer* instance = this;
	render::submit([instance]()
		{
			Application& app = Application::Get();
			GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

			auto vulkan_context = vulkan_context::get();
			auto device = vulkan_context->get_device()->get_vk_device();

			VkDescriptorPool descriptorPool;

			// Create Descriptor Pool
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "ImGui Vulkan impl failed to create descriptor pool!");

			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForVulkan(window, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = vulkan_context::get_vk_instance();
			init_info.PhysicalDevice = vulkan_context->get_device()->get_vk_physical_device();
			init_info.Device = device;
			init_info.QueueFamily = vulkan_context->get_device()->get_physical_device()->GetQueueFamilyIndices().Graphics_family.value();
			init_info.Queue = vulkan_context->get_device()->get_vk_graphics_queue();
			init_info.PipelineCache = nullptr;
			init_info.DescriptorPool = descriptorPool;
			init_info.Allocator = nullptr;
			init_info.MinImageCount = 2;
			auto& swap_chain = vulkan_context->get_vulkan_swap_chain();
			init_info.ImageCount = swap_chain->GetImageCount();
			//init_info.CheckVkResultFn = Utils::VulkanCheckResult;
			ImGui_ImplVulkan_Init(&init_info, swap_chain->get_vk_render_pass());

			// Upload Fonts
			{
				// Use any command queue

				const VkCommandBuffer vk_command_buffer = vulkan_context->get_device()->get_vk_command_buffer(true);
				ImGui_ImplVulkan_CreateFontsTexture(vk_command_buffer);
				vulkan_context->get_device()->flush_command_buffer(vk_command_buffer);

				if (vkDeviceWaitIdle(device) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "VulkanImGuiLayer failed to wait!");

				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}

			const uint32_t frames_in_flight = render::get_frames_in_flight();
			s_imgui_command_buffers.resize(frames_in_flight);
			for (uint32_t i = 0; i < frames_in_flight; ++i)
				s_imgui_command_buffers[i] = vulkan_context->get_device()->create_secondary_command_buffer();
		});

}

void vulkan_imgui_layer::OnDetach()
{
    KB_PROFILE_SCOPE;
	render::submit([]()
		{
            const auto device = vulkan_context::get()->get_device()->get_vk_device();

			if (vkDeviceWaitIdle(device) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "VulkanImGuiLayer failed to wait!");

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();

			ImGui::DestroyContext();
		});
}

void vulkan_imgui_layer::OnUpdate(Timestep ts)
{

}

void vulkan_imgui_layer::OnImGuiRender(Timestep ts)
{

}

void vulkan_imgui_layer::OnEvent(Event& event)
{

}

void vulkan_imgui_layer::Begin()
{
    KB_PROFILE_SCOPE;

    {
		ImGui_ImplVulkan_NewFrame();
    }

    {
		ImGui_ImplGlfw_NewFrame();
    }

    {
		ImGui::NewFrame();
    }

    {
		ImGuizmo::BeginFrame();
    }
}

void vulkan_imgui_layer::End()
{
    KB_PROFILE_SCOPE;

	ImGui::Render();

	auto& swap_chain = vulkan_context::get()->get_vulkan_swap_chain();

	VkClearValue clear_values[2];
	clear_values[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
	clear_values[1].depthStencil = { 1.0f, 0 };

	const u32 width = swap_chain->GetWidth();
    const u32 height = swap_chain->GetHeight();

    const u32 command_buffer_index = swap_chain->GetCurrentBufferIndex();

	VkCommandBufferBeginInfo draw_cmd_buffer_info = {};
	draw_cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	draw_cmd_buffer_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	draw_cmd_buffer_info.pNext = nullptr;

	const VkCommandBuffer draw_command_buffer = swap_chain->GetCurrentDrawCommandBuffer();
	if (vkBeginCommandBuffer(draw_command_buffer, &draw_cmd_buffer_info) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "VulkanImGuiLayer failed to create draw command buffer!");

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = swap_chain->get_vk_render_pass();
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent.width = width;
	render_pass_begin_info.renderArea.extent.height = height;
	render_pass_begin_info.clearValueCount = 2; // Color + depth
	render_pass_begin_info.pClearValues = clear_values;
	render_pass_begin_info.framebuffer = swap_chain->GetCurrentFramebuffer();

	vkCmdBeginRenderPass(draw_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo inheritance_info = {};
	inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritance_info.renderPass = swap_chain->get_vk_render_pass();
	inheritance_info.framebuffer = swap_chain->GetCurrentFramebuffer();

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	cmd_buf_info.pInheritanceInfo = &inheritance_info;

	if (vkBeginCommandBuffer(s_imgui_command_buffers[command_buffer_index], &cmd_buf_info) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "VulkanImGuiLayer failed to begin command buffer!");

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(height);
	viewport.width = static_cast<float>(width);
	viewport.height = -static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(s_imgui_command_buffers[command_buffer_index], 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(s_imgui_command_buffers[command_buffer_index], 0, 1, &scissor);

	ImDrawData* main_draw_data = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_imgui_command_buffers[command_buffer_index]);

	if (vkEndCommandBuffer(s_imgui_command_buffers[command_buffer_index]) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "VulkanImGuiLayer failed to end command buffer!");

	std::vector<VkCommandBuffer> command_buffers;
	command_buffers.push_back(s_imgui_command_buffers[command_buffer_index]);

	vkCmdExecuteCommands(draw_command_buffer, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

	vkCmdEndRenderPass(draw_command_buffer);

	if (vkEndCommandBuffer(draw_command_buffer) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "VulkanImGuiLayer fialed to end draw command buffer!");

    const ImGuiIO& io = ImGui::GetIO();
	(void)io;

	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

} // end namespace kb::render::backend::vk
