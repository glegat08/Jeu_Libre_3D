#pragma once

#include "Scene/Scene.h"
#include "Core/CameraComponent.h"
#include "Core/TrasformComponent.h"
#include "KGR_ImGui.h"
#include "VulkanCore.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "Toolbar.h"
#include "Viewport.h"
#include "Offscreen.h"
#include "Layer.h"
#include "Picker.h"
#include "Serialiser.h"
#include "MenuBar.h"
#include <memory>
#include <filesystem>
#include "UndoManager.h"

namespace KGR
{
    namespace Editor
    {
        /** @brief top-level editor object, owns all panels and the editor scene */
        class Context
        {
        public:
            Context(KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& vulkan);
            /**
             * @brief creates all panels and the offscreen render target
             * @param viewportWidth  initial viewport width in pixels
             * @param viewportHeight initial viewport height in pixels
             * @note call after Vulkan and ImGui are initialised
             */
            void Init(uint32_t viewportWidth, uint32_t viewportHeight);

            /**
             * @brief draws all editor panels
             * @note call between ImGui BeginFrame() and EndFrame()
             */
            void Render();

            /**
             * @brief recreates the offscreen target if the viewport was resized
             * @note call after Render() and before app.Render()
             */
            void HandleResize();

            /**
             * @brief releases Vulkan resources
             * @note call before VulkanCore is destroyed
             */
            void Destroy();

            /**
             * @brief saves the current editor scene to a .kscene file
             * @param path output path
             */
            void SaveScene(const std::filesystem::path& path);

            /**
             * @brief clears the editor scene and reloads it from a .kscene file
             * @param path path to an existing .kscene file
             */
            void LoadScene(const std::filesystem::path& path);

            /** @return the scene currently active (editor scene or runtime clone in play mode) */
            Scene* GetActiveScene() const;

            /** @return the entity currently selected in the hierarchy */
            SceneEntity GetSelectedEntity() const;
            CameraComponent& GetCamera();
            TransformComponent& GetCamTransform();
            Offscreen& GetOffscreen();
            UndoManager& GetUndoManager();
            glm::vec2 GetViewportPos() const;
            glm::vec2 GetViewportSize() const;

        private:
            void RegisterClone();
            void RegisterInspector();
            void RegisterSerializer();
            void HandleUndoRedoShortcuts();

            KGR::_ImGui::ImGuiCore& m_imGui;
            KGR::_Vulkan::VulkanCore& m_vulkan;

            Scene m_editorScene;

            std::unique_ptr<Toolbar> m_toolbar;
            std::unique_ptr<HierarchyPanel> m_hierarchy;
            std::unique_ptr<InspectorPanel> m_inspector;
            std::unique_ptr<Viewport> m_viewport;
            std::unique_ptr<MenuBar> m_menuBar;

            Picker m_picker;
            Offscreen m_offscreen;
            CameraComponent m_camera;
            TransformComponent m_cameraTransform;
            UndoManager m_undoManager;
            Scene* m_lastActiveScene = nullptr;
        };
    }
}