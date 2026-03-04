#include "KGR_ImGui.h"

#include "Backends/imgui_impl_glfw.h"
#include "Backends/imgui_impl_vulkan.h"
#include <Windows.h>
#include "Core/Mesh.h"
#include "Core/CameraComponent.h"

void KGR::_ImGui::ImGuiCore::InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow)
{
	m_VulkanCore = vulkanCore;
	m_Window     = engineWindow;
	IMGUI_CHECKVERSION();

	InitContext(m_EngineContext, vulkanCore, engineWindow);
}

bool KGR::_ImGui::ImGuiCore::LoadMesh(MeshComponent& meshComponent, std::string& path, _Vulkan::VulkanCore& vkCore)
{
	std::string newPath = OpenFile();
	if (newPath.empty())
		return false;

	vkCore.GetDevice().Get().waitIdle();

	if (!path.empty())
		MeshLoader::Unload(path);

	path = newPath;
	meshComponent.mesh = &MeshLoader::Load(path, &vkCore);

	return true;
}

void KGR::_ImGui::ImGuiCore::BeginFrame(ContextTarget target)
{
    SetContext(target);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void KGR::_ImGui::ImGuiCore::SetContext(ContextTarget target)
{
    ImGui::SetCurrentContext(target == ContextTarget::Engine ? m_EngineContext : m_GameContext);
}

void KGR::_ImGui::ImGuiCore::SetWindow(const ImVec2& position, const ImVec2& size, const char* name, bool* p_open)
{
    ImGui::SetNextWindowPos(position, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    ImGui::Begin(name, p_open);
}

void KGR::_ImGui::ImGuiCore::SetCamera(CameraComponent* cam, TransformComponent* transform, float speed)
{
    m_Camera       = cam;
    m_CamTransform = transform;
    m_CamSpeed     = speed;

    glm::vec3 forward = transform->GetLocalAxe<RotData::Dir::Forward>();
    m_Pitch = glm::asin(glm::clamp(forward.y, -1.0f, 1.0f));
    m_Yaw   = std::atan2(forward.x, -forward.z);
}

void KGR::_ImGui::ImGuiCore::UpdateCamera(float deltaTime)
{
    if (!m_Camera || !m_CamTransform || !m_Window)
        return;

    bool rightHeld = glfwGetMouseButton(&m_Window->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (rightHeld)
    {
        glm::dvec2 mousePos;
        glfwGetCursorPos(&m_Window->GetWindow(), &mousePos.x, &mousePos.y);

        if (!m_IsRightClickActive)
        {
            m_LastMousePos       = mousePos;
            m_IsRightClickActive = true;
            glfwSetInputMode(&m_Window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glm::dvec2 delta = mousePos - m_LastMousePos;
            m_LastMousePos   = mousePos;

            m_Yaw   -= glm::radians(static_cast<float>(delta.x)  *m_MouseSensitivity);
            m_Pitch -= glm::radians(static_cast<float>(delta.y)  *m_MouseSensitivity);
            m_Pitch  = glm::clamp(m_Pitch, glm::radians(-89.0f), glm::radians(89.0f));

            glm::quat yawQuat   = glm::angleAxis(m_Yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat pitchQuat = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            m_CamTransform->SetOrientation(yawQuat * pitchQuat);
            m_Camera->UpdateCamera(m_CamTransform->GetFullTransform());
        }
    }
    else if (m_IsRightClickActive)
    {
        m_IsRightClickActive = false;
        glfwSetInputMode(&m_Window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    glm::vec3 move(0.0f);

    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
        move += m_CamTransform->GetLocalAxe<RotData::Dir::Forward>();
    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
        move -= m_CamTransform->GetLocalAxe<RotData::Dir::Forward>();
    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
        move -= m_CamTransform->GetLocalAxe<RotData::Dir::Right>();
    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
        move += m_CamTransform->GetLocalAxe<RotData::Dir::Right>();
    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_Q) == GLFW_PRESS)
        move += glm::vec3(0.0f, 1.0f, 0.0f);
    if (glfwGetKey(&m_Window->GetWindow(), GLFW_KEY_E) == GLFW_PRESS)
        move -= glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::length(move) > 0.0f)
    {
        move = glm::normalize(move) * m_CamSpeed * deltaTime;
        m_CamTransform->Translate(move);
        m_Camera->UpdateCamera(m_CamTransform->GetFullTransform());
    }
}

CameraComponent& KGR::_ImGui::ImGuiCore::GetCam()
{
    return *m_Camera;
}

TransformComponent& KGR::_ImGui::ImGuiCore::GetCamTransform()
{
    return *m_CamTransform;
}

void KGR::_ImGui::ImGuiCore::EndFrame()
{
    ImGui::Render();
}

ImDrawData* KGR::_ImGui::ImGuiCore::GetDrawData()
{
    return ImGui::GetDrawData();
}

ImGuiIO& KGR::_ImGui::ImGuiCore::GetIO()
{
    return ImGui::GetIO();
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

bool KGR::_ImGui::ImGuiCore::IsButton(ButtonType type)
{
    if (type == ButtonType::Object)
        return ImGui::Button("Object");
    else if (type == ButtonType::Light)
        return ImGui::Button("Light");
    else if (type == ButtonType::Camera)
        return ImGui::Button("Camera");
    else if (type == ButtonType::Scene)
        return ImGui::Button("Scene");
    else if (type == ButtonType::Load)
        return ImGui::Button("Load");
	else if (type == ButtonType::PlayAnimation)
		return ImGui::Button("Play Animation");
	else if (type == ButtonType::StopAnimation)
		return ImGui::Button("Stop Animation");
	else if (type == ButtonType::ResetObject)
		return ImGui::Button("Reset Object");

	return false;
}