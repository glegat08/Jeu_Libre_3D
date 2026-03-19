#pragma once
#include "InputManager.h"
#include "ManagerImple.h"
#include "Mesh.h"
#include "Texture.h"
#include "Transform2dComponent.h"
#include "UiComponent.h"
#include "VulkanCore.h"
#include "_GLFW.h"

namespace KGR
{
    /**
     * @brief High‑level window and rendering manager.
     *
     * This class encapsulates:
     * - GLFW window creation and state management
     * - VulkanCore initialization and rendering
     * - registration of cameras, lights, and renderable meshes
     * - input handling through InputManager
     *
     * It acts as the main interface between the application and the Vulkan backend.
     */
    class RenderWindow
    {
    public:

        /**
         * @brief Creates a rendering window and initializes Vulkan.
         *
         * @param size Window size in pixels.
         * @param name Window title.
         * @param GlobResourcesPath Path to global engine resources.
         */
        RenderWindow(glm::ivec2 size, const char* name, const std::filesystem::path& GlobResourcesPath);

        /**
         * @brief Initializes global windowing systems (GLFW, Vulkan loaders, etc.).
         *
         * Must be called once before creating any RenderWindow.
         */
        static void Init();

        /**
         * @brief Destroys the window and releases Vulkan resources.
         */
        void Destroy();

        /**
         * @brief Returns true if the window should close.
         */
        bool ShouldClose() const;

        /**
         * @brief Polls system events (keyboard, mouse, window events).
         *
         * Should be called once per frame.
         */
        static void PollEvent();

        /**
         * @brief Terminates global windowing systems.
         *
         * Must be called once after all windows are destroyed.
         */
        static void End();

        /**
         * @brief Updates internal systems (input, window state, etc.).
         *
         * Should be called once per frame before rendering.
         */
        void Update();

        /**
         * @brief Returns the VulkanCore instance associated with this window.
         */
        _Vulkan::VulkanCore* App();

        /**
         * @brief Returns the current window size.
         */
        glm::ivec2 GetSize() const;

        /**
         * @brief Registers a light component into the Vulkan renderer.
         *
         * @tparam Type Light type (Directional, Point, Spot).
         * @param light Light component.
         * @param transform Transform providing position and orientation.
         */
        template<LightData::Type Type>
        void RegisterLight(LightComponent<Type>& light, TransformComponent& transform);

        /**
         * @brief Registers a camera into the Vulkan renderer.
         *
         * @param cam Camera component.
         * @param transform Transform providing position and orientation.
         */
        void RegisterCam(CameraComponent& cam, TransformComponent& transform);

        /**
         * @brief Registers a mesh + texture pair for rendering.
         *
         * @param mesh Mesh component.
         * @param transform Transform providing world position.
         * @param texture Texture component.
         */
        void RegisterRender(MeshComponent& mesh, TransformComponent& transform, MaterialComponent& material);
        
    	
        void RegisterUi(UiComponent& component, TransformComponent2d& transform, TextureComponent& texture);
    	/**
         * @brief Renders the current frame.
         *
         * @param clearColor Background color.
         * @param imguiDraw Optional ImGui draw data.
         */
        void Render(const glm::vec4& clearColor = { 0,0,0,1 }, ImDrawData* imguiDraw = nullptr);

        /**
         * @brief Changes the window state (fullscreen, windowed, borderless, etc.).
         *
         * @param state Desired window state.
         * @param monitor Optional monitor for fullscreen modes.
         */
        void SetWindowState(_GLFW::WinState state, _GLFW::Monitor* monitor = nullptr);

        /**
         * @brief Checks if the window is currently in a specific state.
         *
         * @tparam state State to test.
         */
        template<_GLFW::WinState state>
        bool IsState() const;

        /**
         * @brief Returns the input manager associated with this window.
         */
        InputManager* GetInputManager();

    private:
        _Vulkan::VulkanCore m_core; ///< Vulkan backend instance.
        KGR::_GLFW::Window m_window; ///< GLFW window wrapper.
        KGR::InputManager m_manager; ///< Input manager for this window.
    };


    // -------------------------------------------------------------------------
    // Template Implementation
    // -------------------------------------------------------------------------

    /**
     * @brief Registers a light into the Vulkan renderer.
     *
     * Converts the LightComponent into LightData, injects transform information,
     * and forwards it to VulkanCore.
     */
    template <LightData::Type Type>
    void RenderWindow::RegisterLight(LightComponent<Type>& light, TransformComponent& transform)
    {
        LightData lightData = light.ToData();
        lightData.pos = transform.GetPosition();
        lightData.dir = transform.GetLocalAxe<RotData::Dir::Forward>();
        m_core.RegisterLight(lightData);
    }

} // namespace KGR