#pragma once

#include "KGR_ImGui.h"
#include "VulkanCore.h"
#include "Scene/Scene.h"
#include "Core/CameraComponent.h"
#include "Picker.h"
#include <ImGuizmo.h>
#include <glm/glm.hpp>

namespace KGR
{
    namespace Editor
    {
        class Context;

        /** @brief the three transform manipulation modes */
        enum class GizmoMode
        { 
            Translate,
            Rotate,
            Scale 
        };

        /** @brief renders the 3D scene into an ImGui window */
        class Viewport
        {
        public:
            Viewport(Context* context, KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& vulkanCore);

            /**
             * @brief draws the viewport window, outlines and gizmo
             * @param picker current selection and hover state
             * @param scene  active scene
             * @param cam    editor camera
             */
            void Render(Picker& picker, Scene* scene, CameraComponent* cam);

            /** @return viewport content size in pixels */
            glm::vec2 GetSize() const { return m_ViewportSize; }

            /** @return viewport top-left corner in screen pixels */
            glm::vec2 GetPos() const { return m_ViewportPos; }

            bool IsFocused() const { return m_IsFocused; }
            bool IsHovered() const { return m_IsHovered; }

            GizmoMode GetGizmoMode() const { return m_GizmoMode; }
            void SetGizmoMode(GizmoMode mode) { m_GizmoMode = mode; }

            /**
             * @brief gives the viewport the ImGui texture handle for the rendered scene
             * @note call after creating or resizing the Offscreen target
             */
            void SetSceneDescriptor(VkDescriptorSet descriptor) { m_sceneDescriptor = descriptor; }

            /**
             * @return true if the viewport was resized this frame
             * @note when true, recreate the Offscreen target and call SetSceneDescriptor again
             */
            bool WasResizedThisFrame() const { return m_WasResized; }

        private:
            void DrawGizmo(SceneEntity e, Scene* scene, CameraComponent* cam);
            void DrawOutlines(const Picker& picker, Scene* scene, CameraComponent* cam);

            Context* m_context = nullptr;
            KGR::_ImGui::ImGuiCore& m_ImGui;
            KGR::_Vulkan::VulkanCore& m_VulkanCore;

            VkDescriptorSet m_sceneDescriptor = VK_NULL_HANDLE;
            glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
            glm::vec2 m_ViewportSizePrev = { 0.0f, 0.0f };
            glm::vec2 m_ViewportPos = { 0.0f, 0.0f };

            bool m_IsFocused = false;
            bool m_IsHovered = false;
            bool m_WasResized = false;

            GizmoMode m_GizmoMode = GizmoMode::Translate;
            bool m_GizmoEditing = false;
            SceneEntity m_GizmoEditedEntity = NullEntity;
            Scene* m_GizmoEditedScene = nullptr;
            TransformComponent m_GizmoInitialTransform;
        };
    }
}