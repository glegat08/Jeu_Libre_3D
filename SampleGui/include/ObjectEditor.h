#pragma once

#include "ObjectState.h"
#include "KGR_ImGui.h"
#include "VulkanCore.h"

class ObjectEditor
{
public:
	ObjectEditor(KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& app)
		: m_ImGui(imgui), m_App(app), m_Obj(nullptr)
	{}

	void SetTarget(ObjectState* obj) { m_Obj = obj; }
	bool Render();
	bool DeleteObject() const { return m_Delete; }
	void DeleteSelected(std::vector<ObjectState>& objects, int& selectedObj);

private:
	KGR::_ImGui::ImGuiCore&   m_ImGui;
	KGR::_Vulkan::VulkanCore& m_App;
	ObjectState*              m_Obj;
	bool                      m_Delete = false;
};
