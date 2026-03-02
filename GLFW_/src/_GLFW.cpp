#include "GLFWFiles.h"
#include "_GLFW.h"

#include <stdexcept>

KGR::_GLFW::Window::Window()
	: m_window(nullptr)
	, m_lasWindowedPos(0, 0)
	, m_lasWindowedSize(0, 0)
{

}



void KGR::_GLFW::Window::Init()
{
	glfwInit();
	
}

void KGR::_GLFW::Window::Destroy()
{
	glfwTerminate();
}

void KGR::_GLFW::Window::PollEvent()
{
	glfwPollEvents();
}

void KGR::_GLFW::Window::AddHint(int hint, int value)
{
	glfwWindowHint(hint, value);
}

const GLFWwindow& KGR::_GLFW::Window::GetWindow() const
{
	return *m_window;
}

GLFWwindow& KGR::_GLFW::Window::GetWindow()
{
	return *m_window;
}

const GLFWwindow* KGR::_GLFW::Window::GetWindowPtr() const
{
	return m_window;
}

GLFWwindow* KGR::_GLFW::Window::GetWindowPtr()
{
	return m_window;
}

void KGR::_GLFW::Window::CreateMyWindow(glm::ivec2 size, const char* name, Monitor* monitor,Window* window)
{
	if (!IsState<WinState::Error>())
		DestroyMyWindow();

	m_info.State = monitor == nullptr ? WinState::Windowed : WinState::FullScreen;
	m_window = glfwCreateWindow(size.x, size.y, name,monitor == nullptr ? nullptr : monitor->glfwMonitor, window == nullptr ? nullptr : &window->GetWindow());

	glfwSetWindowUserPointer(m_window, this);
	glfwSetWindowPosCallback(m_window, &KGR::_GLFW::Window::PosCallBack);
	glfwSetWindowSizeCallback(m_window, &KGR::_GLFW::Window::SizeCallBack);

	glfwGetWindowPos(m_window, &m_info.m_pos.x, &m_info.m_pos.y);
	glfwGetWindowSize(m_window, &m_info.m_size.x, &m_info.m_size.y);
}

void KGR::_GLFW::Window::DestroyMyWindow()
{
	if (!IsState<WinState::Error>())
		glfwDestroyWindow(m_window);

	m_window = nullptr;
	m_info.State = WinState::Error; // Error ? Maybe add "Destroyed" in the enum class

	m_info.m_size = { 0,0 };
	m_info.m_pos = { 0,0 };
}

bool KGR::_GLFW::Window::ShouldClose() const
{
	if (IsState<WinState::Error>())
		throw std::runtime_error("Invalid Window");

	return glfwWindowShouldClose(m_window);
}

void KGR::_GLFW::Window::SetSize(glm::ivec2 size)
{
	if (IsState<WinState::Error>())
		throw std::runtime_error("Resizable only on window mode");

	glfwSetWindowSize(m_window, size.x, size.y);
}

void KGR::_GLFW::Window::SetPos(glm::ivec2 pos)
{
	if (IsState<WinState::Error>())
		throw std::runtime_error("Repositioning only on window mode");

	glfwSetWindowPos(m_window, pos.x, pos.y);
}

void KGR::_GLFW::Window::SetWindowState(WinState state, Monitor* monitor)
{
	switch (state)
	{
	case WinState::Windowed:
		Windowed();
		break;
	case WinState::FullScreen:
		FullScreen(*monitor);
		break;
	case WinState::Error:
		throw std::runtime_error("Invalid Window");
	}
}

void KGR::_GLFW::Window::UpdateParameters()
{
	m_info.m_sizeUpdated = false;
	m_info.m_posUpdated = false;
}

glm::ivec2 KGR::_GLFW::Window::GetPos() const
{
	return m_info.m_pos;
}

glm::ivec2 KGR::_GLFW::Window::GetSize() const
{
	return m_info.m_size;
}

bool KGR::_GLFW::Window::PositionUpdated() const
{
	return m_info.m_posUpdated;
}

bool KGR::_GLFW::Window::SizeUpdated() const
{
	return m_info.m_sizeUpdated;
}

void KGR::_GLFW::Window::PosCallBack(GLFWwindow* window, int posX, int posY)
{
	auto userPointer = static_cast<Window*>(glfwGetWindowUserPointer(window));
	userPointer->m_info.m_pos.x = posX;
	userPointer->m_info.m_pos.y = posY;
	userPointer->m_info.m_posUpdated = true;
}

void KGR::_GLFW::Window::SizeCallBack(GLFWwindow* window, int width, int height)
{
	auto userPointer = static_cast<Window*>(glfwGetWindowUserPointer(window));
	userPointer->m_info.m_size.x = width;
	userPointer->m_info.m_size.y = height;
	userPointer->m_info.m_sizeUpdated = true;
}

void KGR::_GLFW::Window::Windowed()
{
	if (IsState<WinState::Windowed>())
		return;

	m_info.State = WinState::Windowed;

	glfwSetWindowMonitor(

		m_window,
		nullptr,
		m_lasWindowedPos.x,
		m_lasWindowedPos.y,
		m_lasWindowedSize.x,
		m_lasWindowedSize.y,
		0
		
	);
}

void KGR::_GLFW::Window::FullScreen(Monitor monitor)
{
	if (IsState<WinState::FullScreen>())
		return;

	if (!monitor.glfwMonitor)
		throw std::runtime_error("Invalid Monitor");

	m_info.State = WinState::FullScreen;

	m_lasWindowedPos = m_info.m_pos;
	m_lasWindowedSize = m_info.m_size;

	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor.glfwMonitor);

	glfwSetWindowMonitor(
		m_window,
		monitor.glfwMonitor,
		0, 0,
		videoMode->width,
		videoMode->height,
		videoMode->refreshRate
	);
}
