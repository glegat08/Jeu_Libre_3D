#include "KGR_ImGui.h"

#include "Backends/imgui_impl_glfw.h"
#include "Backends/imgui_impl_vulkan.h"

KGR::_ImGui::ImGuiCore::ImGuiCore()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

KGR::_ImGui::ImGuiCore::~ImGuiCore()
{
	Destroy();
}

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
    SetContext(target);

    if (target == ContextTarget::Engine)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("OBJ Loader");

		ImGui::Text("Filepath :");
        ImGui::InputText("##Filepath", m_ObjFilePath, sizeof(m_ObjFilePath));
		ImGui::SameLine();

        if (ImGui::Button("Load"))
        {
            TOLManager::Load(m_ObjFilePath);
        }

        ImGui::Spacing();
    }
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

void KGR::_ImGui::ImGuiCore::InitInfo(ImGuiInitInfo& initInfo)
{
    initInfo.ApiVersion = VK_API_VERSION_1_4;
    initInfo.Instance = Get<VkInstance>(m_VulkanCore->GetInstance());
    initInfo.PhysicalDevice = Get<VkPhysicalDevice>(m_VulkanCore->GetPhysicalDevice());
    initInfo.Device = Get<VkDevice>(m_VulkanCore->GetDevice());
    initInfo.Queue = Get<VkQueue>(m_VulkanCore->GetQueue());
    initInfo.QueueFamily = m_VulkanCore->GetDevice().GetQueueIndex();
    initInfo.DescriptorPool = Get<VkDescriptorPool>(m_VulkanCore->GetDescriptorPool());
    initInfo.MinImageCount = m_VulkanCore->GetSwapChain().GetImagesCount();
    initInfo.ImageCount = m_VulkanCore->GetSwapChain().GetImagesCount();
    initInfo.UseDynamicRendering = true;

    initInfo.ColorFormat = static_cast<VkFormat>(m_VulkanCore->GetSwapChain().GetFormat().format);
    initInfo.DepthFormat = static_cast<VkFormat>(m_VulkanCore->GetPhysicalDevice().findSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment));

    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &initInfo.ColorFormat,
        .depthAttachmentFormat = initInfo.DepthFormat
    };
}

void KGR::_ImGui::ImGuiCore::InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore,
                                         KGR::_GLFW::Window* window)
{
	context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(&window->GetWindow(), true);
    
    ImGuiInitInfo initInfo;
    InitInfo(initInfo);

    ImGui_ImplVulkan_Init((ImGui_ImplVulkan_InitInfo*)&initInfo);
}
