#pragma once
#include "glfw/glfw3.h"
#include "GLFW/glfw3native.h"
#include <glm/glm.hpp>
#include <ranges>
#include <thread>

namespace KGR
{
    namespace _GLFW
    {
        /**
         * @brief Window state modes (windowed or fullscreen).
         */
        enum class WinState
        {
            Error,
            Windowed,
            FullScreen
        };

        /**
         * @brief Stores window position, size, and state.
         *
         * Updated by GLFW callbacks and used to detect changes.
         */
        struct WinInfo
        {
            glm::ivec2 m_pos = { 0, 0 };   ///< Window position.
            bool m_posUpdated = false;   ///< True if position changed.

            glm::ivec2 m_size = { 0, 0 };  ///< Window size.
            bool m_sizeUpdated = false;  ///< True if size changed.

            WinState State = WinState::Error; ///< Current window state.
        };

        /**
         * @brief Monitor selection mode.
         */
        enum class MonitorType
        {
            Primary, ///< Always returns the primary monitor.
            Current  ///< Returns the monitor closest to the window.
        };

        /**
         * @brief Wrapper for a GLFW monitor pointer.
         */
        struct Monitor
        {
            GLFWmonitor* glfwMonitor = nullptr;
        };

        /**
         * @brief Wrapper around a GLFW window with state management.
         *
         * Provides:
         * - window creation/destruction
         * - fullscreen/windowed switching
         * - size/position tracking
         * - monitor selection
         * - event polling
         *
         * This class abstracts GLFW into a clean engine‑friendly interface.
         */
        struct Window
        {
            Window();

            /**
             * @brief Initializes GLFW (must be called once before creating windows).
             */
            static void Init();

            /**
             * @brief Terminates GLFW (must be called once after all windows are destroyed).
             */
            static void Destroy();

            /**
             * @brief Polls system events (keyboard, mouse, window events).
             */
            static void PollEvent();

            /**
             * @brief Adds a GLFW window hint before creation.
             */
            static void AddHint(int hint, int value);

            /// @brief Returns a const reference to the GLFW window.
            const GLFWwindow& GetWindow() const;

            /// @brief Returns a reference to the GLFW window.
            GLFWwindow& GetWindow();

            /// @brief Returns a const pointer to the GLFW window.
            const GLFWwindow* GetWindowPtr() const;

            /// @brief Returns a pointer to the GLFW window.
            GLFWwindow* GetWindowPtr();

            /**
             * @brief Creates a window with the given size, name, and monitor.
             *
             * @param size Window size.
             * @param name Window title.
             * @param monitor Monitor to attach to (nullptr for windowed).
             * @param window Shared context window (optional).
             */
            void CreateMyWindow(glm::ivec2 size, const char* name, Monitor* monitor, Window* window);

            /**
             * @brief Destroys the GLFW window.
             */
            void DestroyMyWindow();

            /**
             * @brief Returns true if the window should close.
             */
            bool ShouldClose() const;

            /**
             * @brief Sets the window size.
             */
            void SetSize(glm::ivec2 size);

            /**
             * @brief Sets the window position.
             */
            void SetPos(glm::ivec2 pos);

            /**
             * @brief Changes the window state (fullscreen/windowed).
             *
             * @param state Desired state.
             * @param monitor Target monitor (optional).
             */
            void SetWindowState(WinState state, Monitor* monitor = nullptr);

            /**
             * @brief Updates internal position/size info from GLFW callbacks.
             */
            void UpdateParameters();

            /// @brief Returns the window position.
            glm::ivec2 GetPos() const;

            /// @brief Returns the window size.
            glm::ivec2 GetSize() const;

            /// @brief Returns true if the position changed since last update.
            bool PositionUpdated() const;

            /// @brief Returns true if the size changed since last update.
            bool SizeUpdated() const;

            /**
             * @brief Checks if the window is in a specific state.
             *
             * @tparam state State to test.
             */
            template<WinState state>
            bool IsState() const
            {
                return m_info.State == state;
            }

            /**
             * @brief Returns a monitor based on the selection mode.
             *
             * @tparam state Monitor selection mode.
             */
            template<MonitorType state>
            Monitor GetMonitor() const
            {
                if constexpr (state == MonitorType::Primary)
                {
                    return { glfwGetPrimaryMonitor() };
                }
                else
                {
                    int monitorCount = 0;
                    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

                    int finalIndex = 0;
                    float minDistance = std::numeric_limits<float>::max();

                    glm::vec2 windowMiddlePoint = m_info.m_pos + m_info.m_size / 2;

                    for (int i = 0; i < monitorCount; ++i)
                    {
                        GLFWmonitor* current = monitors[i];

                        glm::ivec2 monitorPos;
                        glfwGetMonitorPos(current, &monitorPos.x, &monitorPos.y);

                        glm::vec2 monitorFloatPosition = { monitorPos.x, monitorPos.y };
                        const GLFWvidmode* videoMode = glfwGetVideoMode(current);

                        glm::vec2 monitorSize = { videoMode->width, videoMode->height };
                        glm::vec2 monitorMiddlePoint = monitorFloatPosition + monitorSize / 2.0f;

                        float distance = glm::length(windowMiddlePoint - monitorMiddlePoint);

                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            finalIndex = i;
                        }
                    }

                    return Monitor{ monitors[finalIndex] };
                }
            }

        private:

            /**
             * @brief GLFW callback for window position changes.
             */
            static void PosCallBack(GLFWwindow* window, int posX, int posY);

            /**
             * @brief GLFW callback for window size changes.
             */
            static void SizeCallBack(GLFWwindow* window, int width, int height);

            /// @brief Switches to windowed mode.
            void Windowed();

            /// @brief Switches to fullscreen mode on the given monitor.
            void FullScreen(Monitor monitor);

            GLFWwindow* m_window = nullptr; ///< Raw GLFW window pointer.
            WinInfo m_info;                 ///< Window state information.

            glm::ivec2 m_lasWindowedPos = { 0, 0 }; ///< Last windowed position.
            glm::ivec2 m_lasWindowedSize = { 0, 0 }; ///< Last windowed size.
        };

    } // namespace _GLFW
} // namespace KGR