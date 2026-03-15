#include "Core/Window.h"

#include "InputManager.h"
#include "Core/CameraComponent.h"

KGR::RenderWindow::RenderWindow(glm::ivec2 size, const char* name, const std::filesystem::path& GlobResourcesPath)
{
	STBManager::SetGlobalFIlePath(GlobResourcesPath);
	MeshLoader::SetGlobalFIlePath(GlobResourcesPath);
	TextureLoader::SetGlobalFIlePath(GlobResourcesPath);
	FileManager::SetGlobalFIlePath(GlobResourcesPath);


	m_window.CreateMyWindow(size, name, nullptr, nullptr);
	m_core.initVulkan(&m_window.GetWindow());
	m_manager.Initialize(&m_window.GetWindow());

}

void KGR::RenderWindow::Init()
{
	KGR::_GLFW::Window::Init();
	KGR::_GLFW::Window::AddHint(GLFW_CLIENT_API, GLFW_NO_API);
	KGR::_GLFW::Window::AddHint(GLFW_RESIZABLE, GLFW_TRUE);
}

void KGR::RenderWindow::Destroy()
{
	m_window.DestroyMyWindow();
	STBManager::UnloadAll();
	MeshLoader::UnloadAll();
	TextureLoader::UnloadAll();
	FileManager::UnloadAll();
}

bool KGR::RenderWindow::ShouldClose() const
{
	return m_window.ShouldClose();
}

void KGR::RenderWindow::PollEvent()
{
	KGR::_GLFW::Window::PollEvent();
}

void KGR::RenderWindow::End()
{
	KGR::_GLFW::Window::Destroy();
}

void KGR::RenderWindow::Update()
{
	m_manager.Update();
}

KGR::_Vulkan::VulkanCore* KGR::RenderWindow::App()
{
	return &m_core;
}

glm::ivec2 KGR::RenderWindow::GetSize() const
{
	return m_window.GetSize();
}

void KGR::RenderWindow::RegisterCam(CameraComponent& cam, TransformComponent& transform)
{
	m_core.RegisterCam(transform.GetFullTransform(),cam.GetView(),cam.GetProj());
}

void KGR::RenderWindow::RegisterRender(MeshComponent& mesh, TransformComponent& transform, TextureComponent& texture)
{
	m_core.RegisterRender(*mesh.mesh, transform.GetFullTransform(), texture.GetAllTextures());
}

void KGR::RenderWindow::RegisterUi(UiComponent& component, TransformComponent2d& transform, TextureComponent& texture)
{
	float aspectRatio = static_cast<float>(GetSize().x) / static_cast<float>(GetSize().y);
	transform.SetPosition(component.GetPosNdc(aspectRatio));
	transform.SetScale(component.GetScaleNdc(aspectRatio));
	m_core.RegisterUi(UiData{ component.GetColor(),transform.GetFullTransform() }, texture.GetTexture(0), GetSize());
}

void KGR::RenderWindow::Render(const glm::vec4& clearColor, ImDrawData* imguiDraw)
{
	m_core.Render(&m_window.GetWindow(), clearColor, imguiDraw);
}

void KGR::RenderWindow::SetWindowState(_GLFW::WinState state, _GLFW::Monitor* monitor)
{
	m_window.SetWindowState(state, monitor);
}

KGR::InputManager* KGR::RenderWindow::GetInputManager()
{
	return &m_manager;
}

template <KGR::_GLFW::WinState state>
bool KGR::RenderWindow::IsState() const
{
	return m_window.IsState<state>();
}
