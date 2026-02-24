#pragma once

/**
 * @brief Base event type for the event system.
 *
 * All events inherit from this struct. It provides a virtual destructor
 * to allow polymorphic deletion and a handled flag to stop propagation.
 */
struct Event
{
    virtual ~Event() = default;

    /**
     * @brief Indicates whether the event has been handled.
     *
     * Event dispatchers may set this to true to stop propagation.
     */
    bool m_handled = false;
};

/**
 * @brief Event representing a keyboard input.
 */
struct KeyEvent : public Event
{
    /**
     * @brief Constructs a KeyEvent.
     *
     * @param key Key code associated with the event.
     * @param action Action performed (press, release, repeat).
     */
    KeyEvent(int key, int action) : m_key(key), m_action(action)
    {
    }

    int m_key;     ///< Key code.
    int m_action;  ///< Action type (press/release).
};

/**
 * @brief Event representing mouse movement.
 */
struct MouseEvent : public Event
{
    /**
     * @brief Constructs a MouseEvent.
     *
     * @param x Mouse X position.
     * @param y Mouse Y position.
     */
    MouseEvent(double x, double y) : xPos(x), yPos(y)
    {
    }

    double xPos; ///< Horizontal mouse position.
    double yPos; ///< Vertical mouse position.
};

/**
 * @brief Event representing a mouse button action.
 */
struct MouseButtonEvent : public Event
{
    /**
     * @brief Constructs a MouseButtonEvent.
     *
     * @param button Mouse button index.
     * @param action Action performed (press/release).
     */
    MouseButtonEvent(int button, int action) : m_button(button), m_action(action) 
    {
    }

    int m_button; ///< Mouse button index.
    int m_action; ///< Action type (press/release).
};

/**
 * @brief Event representing mouse scroll input.
 */
struct ScrollEvent : public Event
{
    /**
     * @brief Constructs a ScrollEvent.
     *
     * @param x Horizontal scroll offset.
     * @param y Vertical scroll offset.
     */
    ScrollEvent(double x, double y) : xOffset(x), yOffset(y)
    {
    }

    double xOffset; ///< Horizontal scroll offset.
    double yOffset; ///< Vertical scroll offset.
};

/**
 * @brief Event representing a window resize.
 */
struct WindowResizeEvent : public Event
{
    /**
     * @brief Constructs a WindowResizeEvent.
     *
     * @param width New window width.
     * @param height New window height.
     */
    WindowResizeEvent(int width, int height) : m_width(width), m_height(height)
    {
    }

    int m_width;  ///< New width of the window.
    int m_height; ///< New height of the window.
};

/**
 * @brief Event representing a window close request.
 */
struct WindowCloseEvent : public Event
{
    /**
     * @brief Constructs a WindowCloseEvent.
     */
    WindowCloseEvent() = default;
};

/**
 * @brief Event representing a framebuffer resize.
 *
 * This is often distinct from window resize in APIs like Vulkan or OpenGL.
 */
struct FramebufferResizeEvent : public Event
{
    /**
     * @brief Constructs a FramebufferResizeEvent.
     *
     * @param width New framebuffer width.
     * @param height New framebuffer height.
     */
    FramebufferResizeEvent(int width, int height) : m_width(width), m_height(height)
    {
    }

    int m_width;  ///< New framebuffer width.
    int m_height; ///< New framebuffer height.
};