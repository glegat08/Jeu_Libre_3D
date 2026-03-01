#include "KGR_ImGui.h"

#include "Backends/imgui_impl_glfw.h"
#include "Backends/imgui_impl_vulkan.h"
#include <Windows.h>

void KGR::_ImGui::ImGuiCore::InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow)
{
	m_VulkanCore = vulkanCore;
	IMGUI_CHECKVERSION();

	InitContext(m_EngineContext, vulkanCore, engineWindow);
}

void KGR::_ImGui::ImGuiCore::BeginFrame(ContextTarget target)
{
    SetContext(target);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void KGR::_ImGui::ImGuiCore::EndFrame(ContextTarget target, VkCommandBuffer commandBuffer)
{
    SetContext(target);
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void KGR::_ImGui::ImGuiCore::Render(ContextTarget target)
{
}

void KGR::_ImGui::ImGuiCore::SetContext(ContextTarget target)
{
    ImGui::SetCurrentContext(target == ContextTarget::Engine ? m_EngineContext : m_GameContext);
}

void KGR::_ImGui::ImGuiCore::Destroy()
{
    ImGui::SetCurrentContext(m_EngineContext);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(m_EngineContext);
}

std::string KGR::_ImGui::ImGuiCore::OpenFile()
{
    OPENFILENAMEA ofn = {};
	char path[512]    = "";
	ofn.lStructSize   = sizeof(ofn);
	ofn.lpstrFilter   = "OBJ Files\0*.obj\0All Files\0*.*\0";
	ofn.lpstrFile     = path;
	ofn.nMaxFile      = sizeof(path);
	ofn.Flags         = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt   = "obj";

	return GetOpenFileNameA(&ofn) ? std::string(path) : std::string();
}

void KGR::_ImGui::ImGuiCore::InitInfo()
{
    m_InitInfo.ApiVersion          = VK_API_VERSION_1_4;
    m_InitInfo.Instance            = Get<VkInstance>(m_VulkanCore->GetInstance());
    m_InitInfo.PhysicalDevice      = Get<VkPhysicalDevice>(m_VulkanCore->GetPhysicalDevice());
    m_InitInfo.Device              = Get<VkDevice>(m_VulkanCore->GetDevice());
    m_InitInfo.Queue               = Get<VkQueue>(m_VulkanCore->GetQueue());
    m_InitInfo.QueueFamily         = m_VulkanCore->GetDevice().GetQueueIndex();
    m_InitInfo.DescriptorPool      = Get<VkDescriptorPool>(m_VulkanCore->GetDescriptorPool());
    m_InitInfo.MinImageCount       = m_VulkanCore->GetSwapChain().GetImagesCount();
    m_InitInfo.ImageCount          = m_VulkanCore->GetSwapChain().GetImagesCount();
    m_InitInfo.UseDynamicRendering = true;

    VkFormat ColorFormat = static_cast<VkFormat>(m_VulkanCore->GetSwapChain().GetFormat().format);
    VkFormat DepthFormat = static_cast<VkFormat>(m_VulkanCore->GetPhysicalDevice().findSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment));

    m_InitInfo.PipelineInfoMain.PipelineRenderingCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &ColorFormat,
        .depthAttachmentFormat = DepthFormat
    };

    ImGui_ImplVulkan_Init(&m_InitInfo);
}

void KGR::_ImGui::ImGuiCore::InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore,
                                         KGR::_GLFW::Window* window)
{
	context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(&window->GetWindow(), true);
    
    InitInfo();
}
