#include "Core/InputManager.h"

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/detail/type_vec2.hpp>

namespace KGR
{
    /**
     * @brief Initializes the input manager with the given GLFW window.
     *
     * The window pointer is required for polling keyboard, mouse buttons,
     * and cursor position each frame.
     *
     * @param window GLFW window to read input from.
     */
    void InputManager::Initialize(GLFWwindow* window)
    {
        m_window = window;
    }

    /**
     * @brief Updates the input state for the current frame.
     *
     * - Copies current key/mouse states into the "previous" buffers.
     * - Polls GLFW for the new key/mouse states.
     * - Retrieves the current mouse cursor position.
     *
     * Must be called once per frame before any input queries.
     */
    void InputManager::Update()
    {
        // Copy previous state and Poll current state
        for (int i = 0; i < 1024; i++)
        {
	        m_previousKeys[i] = m_currentKeys[i];
            m_currentKeys[i] = glfwGetKey(m_window, i) == GLFW_PRESS;
        }

        for (int i = 0; i < 8; i++)
        {
	        m_previousMouse[i] = m_currentMouse[i];
            m_currentMouse[i] = glfwGetMouseButton(m_window, i) == GLFW_PRESS;

        }
        // Update cursor position
        oldPos = { m_mouseX,m_mouseY};
        glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
    }

    // -------------------------------------------------------------------------
    // Keyboard Queries
    // -------------------------------------------------------------------------

    /**
     * @brief Returns true if the key is currently held down.
     */
    bool InputManager::IsKDown(int key)
    {
        return m_currentKeys[key];
    }

    /**
     * @brief Returns true only on the frame the key transitions from up to down.
     */
    bool InputManager::IsKPressed(int key)
    {
        return m_currentKeys[key] && !m_previousKeys[key];
    }

    /**
     * @brief Returns true only on the frame the key transitions from down to up.
     */
    bool InputManager::IsKReleased(int key)
    {
        return !m_currentKeys[key] && m_previousKeys[key];
    }

    // -------------------------------------------------------------------------
    // Mouse Queries
    // -------------------------------------------------------------------------

    /**
     * @brief Returns true if the mouse button is currently held down.
     */
    bool InputManager::IsMDown(int button)
    {
        return m_currentMouse[button];
    }

    /**
     * @brief Returns true only on the frame the mouse button is pressed.
     */
    bool InputManager::IsMPressed(int button)
    {
        return m_currentMouse[button] && !m_previousMouse[button];
    }

    /**
     * @brief Returns true only on the frame the mouse button is released.
     */
    bool InputManager::IsMReleased(int button)
    {
        return !m_currentMouse[button] && m_previousMouse[button];
    }

    /**
     * @brief Retrieves the current mouse cursor position.
     *
     * @param x Output X coordinate.
     * @param y Output Y coordinate.
     */
    glm::vec2 InputManager::GetMousePosition() const 
    {
        return {m_mouseX,m_mouseY};
    }

    glm::vec2 InputManager::GetMouseDelta() const
    {
        glm::vec2 result;
        result.x = (m_mouseX - oldPos.x);
        result.y = (m_mouseY - oldPos.y);
        return result;
    }
}
