#pragma once

#include "ObjectState.h"
#include "KGR_ImGui.h"
#include "VulkanCore.h"

/// @brief ImGui panel that exposes transform, animation, and mesh controls for a selected ObjectState.
///
/// Call SetTarget() to bind an object, then call Render() each frame.
/// Check DeleteObject() after Render() to know if the user requested deletion,
/// and call DeleteSelected() to actually remove the object from the scene list.
class ObjectEditor
{
public:
	/// @param imgui Reference to the engine ImGui wrapper.
	/// @param app   Reference to the Vulkan backend (needed for GPU sync on deletion).
	ObjectEditor(KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& app)
		: m_ImGui(imgui), m_App(app), m_Obj(nullptr)
	{
	}

	/// @brief Sets the object to inspect and edit.
	/// @param obj Pointer to the target ObjectState, or nullptr to deselect.
	void SetTarget(ObjectState* obj) { m_Obj = obj; }

	/// @brief Draws the editor panel for the current target.
	/// @return true if the panel is still open, false if the user closed it.
	bool Render();

	/// @brief Returns whether the user clicked the Delete button during the last Render() call.
	/// @return true if deletion was requested.
	bool DeleteObject() const { return m_Delete; }

	/// @brief Removes the selected object from the list, unloading its mesh if not shared.
	/// @param objects     The scene's object list.
	/// @param selectedObj Index of the selected object; reset to -1 after deletion.
	void DeleteSelected(std::vector<ObjectState>& objects, int& selectedObj);

private:
	KGR::_ImGui::ImGuiCore& m_ImGui;
	KGR::_Vulkan::VulkanCore& m_App;
	ObjectState* m_Obj;
	bool m_Delete = false;
};