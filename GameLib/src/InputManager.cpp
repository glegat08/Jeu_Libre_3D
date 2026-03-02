#include "InputManager.h"

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
        // Copy previous state
        for (int i = 0; i < 1024; i++)
            m_previousKeys[i] = m_currentKeys[i];

        for (int i = 0; i < 8; i++)
            m_previousMouse[i] = m_currentMouse[i];

        // Poll current keyboard state
        for (int key = 0; key < 1024; key++)
            m_currentKeys[key] = glfwGetKey(m_window, key) == GLFW_PRESS;

        // Poll current mouse button state
        for (int mb = 0; mb < 8; mb++)
            m_currentMouse[mb] = glfwGetMouseButton(m_window, mb) == GLFW_PRESS;

        // Update cursor position
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
    void InputManager::GetMousePosition(double& x, double& y)
    {
        x = m_mouseX;
        y = m_mouseY;
    }
}