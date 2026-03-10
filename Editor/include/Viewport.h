#pragma once

#include "KGR_ImGui.h"
#include "VulkanCore.h"
#include "Scene/Scene.h"
#include "Core/CameraComponent.h"
#include <ImGuizmo.h>
#include <glm/glm.hpp>

namespace KGR
{
	namespace Editor
	{
		/// @brief Gizmo manipulation mode.
		enum class GizmoMode
		{
			Translate,
			Rotate,
			Scale
		};

		/// @brief Renders the 3D scene into an ImGui window, similar to an Unreal Engine viewport.
		///
		/// The viewport captures the scene camera's output and displays it as an ImGui image.
		/// It handles resizing, aspect ratio, gizmo rendering, and provides the content region
		/// size for the renderer.
		class Viewport
		{
		public:
			/// @param imgui     Reference to the ImGui wrapper.
			/// @param vulkanCore Reference to the Vulkan backend.
			Viewport(KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& vulkanCore);

			/// @brief Renders the viewport ImGui window with optional gizmo.
			/// @param selectedEntity Entity to show the gizmo on (NullEntity to hide).
			/// @param scene          Active scene (for reading/writing transforms).
			/// @param cam            Editor camera (for view/proj matrices).
			void Render(SceneEntity selectedEntity, Scene* scene, CameraComponent* cam);

			/// @brief Returns the current viewport content size in pixels.
			glm::vec2 GetSize() const { return m_ViewportSize; }

			/// @brief Returns true if the viewport window is focused.
			bool IsFocused() const { return m_IsFocused; }

			/// @brief Returns true if the mouse is hovering the viewport.
			bool IsHovered() const { return m_IsHovered; }

			/// @brief Sets the gizmo mode (Translate / Rotate / Scale).
			void SetGizmoMode(GizmoMode mode) { m_GizmoMode = mode; }

			/// @brief Returns the current gizmo mode.
			GizmoMode GetGizmoMode() const { return m_GizmoMode; }

		private:
			/// @brief Draws the ImGuizmo gizmo over the viewport for the selected entity.
			void DrawGizmo(SceneEntity e, Scene* scene, CameraComponent* cam);

			KGR::_ImGui::ImGuiCore&      m_ImGui;
			KGR::_Vulkan::VulkanCore&    m_VulkanCore;

			glm::vec2  m_ViewportSize = { 1280.0f, 720.0f };
			glm::vec2  m_ViewportPos  = { 0.0f, 0.0f };
			bool       m_IsFocused    = false;
			bool       m_IsHovered    = false;
			GizmoMode  m_GizmoMode    = GizmoMode::Translate;
		};
	}
}
