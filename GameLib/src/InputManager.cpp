#include "InputManager.h"

namespace KGR
{
	void InputManager::Initialize(GLFWwindow* window)
	{
		m_window = window;
	}

	void InputManager::Update()
	{
		//Update previous state
		for (int i = 0; i < 1024; i++)
			m_previousKeys[i] = m_currentKeys[i];
		for (int i = 0; i < 8; i++)
			m_previousMouse[i] = m_currentMouse[i];

		//Update current state
		for (int key = 0; key < 1024; key++)
			m_currentKeys[key] = glfwGetKey(m_window, key) == GLFW_PRESS;
		for (int mb = 0; mb < 8; mb++)
			m_currentMouse[mb] = glfwGetMouseButton(m_window, mb) == GLFW_PRESS;

		//update mouse position
		glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
	}

	//KEYBOARD

	bool InputManager::IsKDown(int key)
	{
		return m_currentKeys[key];
	}

	bool InputManager::IsKPressed(int key)
	{
		return m_currentKeys[key] && !m_previousKeys[key];
	}

	bool InputManager::IsKReleased(int key)
	{
		return !m_currentKeys[key] && m_previousKeys[key];
	}

	//MOUSE

	bool InputManager::IsMDown(int button)
	{
		return m_currentMouse[button];
	}

	bool InputManager::IsMPressed(int button)
	{
		return m_currentMouse[button] && !m_previousMouse[button];
	}

	bool InputManager::IsMReleased(int button)
	{
		return !m_currentMouse[button] && m_previousMouse[button];
	}

	void InputManager::GetMousePosition(double& x, double& y)
	{
		x = m_mouseX;
		y = m_mouseY;
	}
}