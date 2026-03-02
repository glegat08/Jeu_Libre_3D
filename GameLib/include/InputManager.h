#pragma once
#include <GLFW/glfw3.h>
#include <type_traits>

namespace KGR
{
    /**
     * @brief Concept ensuring that a type is an enum usable as an input identifier.
     */
    template <typename T>
    concept InputEnum = std::is_enum_v<T>;

    /**
     * @brief Keyboard keys mapped to GLFW key codes.
     *
     * These values intentionally map to GLFW's key constants so they can be
     * passed directly to GLFW input functions.
     */
    enum class Key
    {
        A = GLFW_KEY_Q,
        B = GLFW_KEY_B,
        C = GLFW_KEY_C,
        D = GLFW_KEY_D,
        E = GLFW_KEY_E,
        F = GLFW_KEY_F,
        G = GLFW_KEY_G,
        H = GLFW_KEY_H,
        I = GLFW_KEY_I,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        L = GLFW_KEY_L,
        M = GLFW_KEY_M,
        N = GLFW_KEY_N,
        O = GLFW_KEY_O,
        P = GLFW_KEY_P,
        Q = GLFW_KEY_A,
        R = GLFW_KEY_R,
        S = GLFW_KEY_S,
        T = GLFW_KEY_T,
        U = GLFW_KEY_U,
        V = GLFW_KEY_V,
        W = GLFW_KEY_Z,
        X = GLFW_KEY_X,
        Y = GLFW_KEY_Y,
        Z = GLFW_KEY_W,

        Num0 = GLFW_KEY_0,
        Num1 = GLFW_KEY_1,
        Num2 = GLFW_KEY_2,
        Num3 = GLFW_KEY_3,
        Num4 = GLFW_KEY_4,
        Num5 = GLFW_KEY_5,
        Num6 = GLFW_KEY_6,
        Num7 = GLFW_KEY_7,
        Num8 = GLFW_KEY_8,
        Num9 = GLFW_KEY_9,
    };

    /**
     * @brief Special non-character keys.
     */
    enum class SpecialKey
    {
        Space = GLFW_KEY_SPACE,
        Escape = GLFW_KEY_ESCAPE,
        Enter = GLFW_KEY_ENTER,
        Tab = GLFW_KEY_TAB,
        Shift = GLFW_KEY_LEFT_SHIFT,
        Ctrl = GLFW_KEY_LEFT_CONTROL,
        Alt = GLFW_KEY_LEFT_ALT
    };

    /**
     * @brief Mouse buttons mapped to GLFW button codes.
     */
    enum class Mouse
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE,
        Last = GLFW_MOUSE_BUTTON_LAST,
        Button1 = GLFW_MOUSE_BUTTON_1,
        Button2 = GLFW_MOUSE_BUTTON_2,
        Button3 = GLFW_MOUSE_BUTTON_3,
        Button4 = GLFW_MOUSE_BUTTON_4,
        Button5 = GLFW_MOUSE_BUTTON_5,
        Button6 = GLFW_MOUSE_BUTTON_6,
        Button7 = GLFW_MOUSE_BUTTON_7,
        Button8 = GLFW_MOUSE_BUTTON_8
    };

    /**
     * @brief Manages keyboard and mouse input using GLFW.
     *
     * Tracks current and previous input states to detect:
     * - key/button down
     * - key/button pressed (edge)
     * - key/button released (edge)
     *
     * Also retrieves mouse position each frame.
     */
    class InputManager
    {
    public:
        /**
         * @brief Initializes the input manager with a GLFW window.
         *
         * @param window Pointer to the GLFW window to poll input from.
         */
        void Initialize(GLFWwindow* window);

        /**
         * @brief Updates input states.
         *
         * Must be called once per frame to refresh keyboard/mouse states.
         */
        void Update();

        bool IsKDown(int key);
        bool IsKPressed(int key);
        bool IsKReleased(int key);

        bool IsMDown(int button);
        bool IsMPressed(int button);
        bool IsMReleased(int button);

        /**
         * @brief Retrieves the current mouse cursor position.
         *
         * @param x Output X coordinate.
         * @param y Output Y coordinate.
         */
        void GetMousePosition(double& x, double& y);


        template<InputEnum T>
        bool IsKeyDown(T key)
        {
            return IsKDown(static_cast<int>(key));
        }

        template<InputEnum T>
        bool IsKeyPressed(T key)
        {
            return IsKPressed(static_cast<int>(key));
        }

        template<InputEnum T>
        bool IsKeyReleased(T key)
        {
            return IsKReleased(static_cast<int>(key));
        }

        template<InputEnum T>
        bool IsMousePressed(T key)
        {
            return IsMPressed(static_cast<int>(key));
        }

        template<InputEnum T>
        bool IsMouseReleased(T key)
        {
            return IsMReleased(static_cast<int>(key));
        }

        template<InputEnum T>
        bool IsMouseDown(T key)
        {
            return IsMDown(static_cast<int>(key));
        }

    private:
        GLFWwindow* m_window = nullptr;         ///< Associated GLFW window.

        bool m_currentKeys[1024] = { false };   ///< Current frame keyboard state.
        bool m_previousKeys[1024] = { false };  ///< Previous frame keyboard state.

        bool m_currentMouse[8] = { false };     ///< Current frame mouse state.
        bool m_previousMouse[8] = { false };    ///< Previous frame mouse state.

        double m_mouseX = 0.0;                  ///< Current mouse X position.
        double m_mouseY = 0.0;                  ///< Current mouse Y position.
    };
}