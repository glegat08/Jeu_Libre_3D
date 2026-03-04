#include "ObjectEditor.h"
#include "Core/ManagerImple.h"
#include <filesystem>

bool ObjectEditor::Render()
{
	if (!m_Obj)
		return false;

	m_Delete = false;
	bool open = true;

	KGR::_ImGui::ImGuiCore::SetWindow({ 20, 20 }, { 600, 250 }, "Object Editor", &open);

	char nameBuf[64] = {};
	strncpy_s(nameBuf, m_Obj->name.c_str(), sizeof(nameBuf) - 1);

	if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)) && nameBuf[0] != '\0')
		m_Obj->name = nameBuf;

	ImGui::Separator();
	ImGui::Text("File : %s", m_Obj->modelName.c_str());

	if (m_ImGui.IsButton(KGR::_ImGui::ButtonType::Load))
	{
		if (m_ImGui.LoadMesh(m_Obj->mesh, m_Obj->modelPath, m_App))
		{
			m_Obj->modelName = std::filesystem::path(m_Obj->modelPath).filename().string();
			m_Obj->ResetTransform();
		}
	}

	if (!m_Obj->isAnimating)
	{
		if (m_ImGui.IsButton(KGR::_ImGui::ButtonType::PlayAnimation))
			m_Obj->isAnimating = true;
	}
	else
	{
		if (m_ImGui.IsButton(KGR::_ImGui::ButtonType::StopAnimation))
			m_Obj->isAnimating = false;
	}

	if (m_ImGui.IsButton(KGR::_ImGui::ButtonType::ResetObject))
		m_Obj->ResetTransform();

	ImGui::Separator();

	ImGui::DragFloat3("Position",	 &m_Obj->position.x, 0.05f);
	ImGui::DragFloat3("Scale",		 &m_Obj->scale.x,    0.05f, 0.01f, 50.0f);
	ImGui::SliderAngle("Pitch (x)", &m_Obj->rotation.x, -180.0f, 180.0f);
	ImGui::SliderAngle("Yaw (y)",	 &m_Obj->rotation.y, -180.0f, 180.0f);
	ImGui::SliderAngle("Roll (z)",	 &m_Obj->rotation.z, -180.0f, 180.0f);

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.5f, 0.0f, 0.0f, 1.0f));

	if (ImGui::Button("Delete Object", ImVec2(-1, 0)))
		m_Delete = true;

	ImGui::PopStyleColor(3);

	ImGui::End();

	return open;
}

void ObjectEditor::DeleteSelected(std::vector<ObjectState>& objects, int& selectedObj)
{
	m_App.GetDevice().Get().waitIdle();

	if (!objects[selectedObj].modelPath.empty())
	{
		bool sharedMesh = false;
		for (int j = 0; j < static_cast<int>(objects.size()); j++)
		{
			if (j != selectedObj && objects[j].modelPath == objects[selectedObj].modelPath)
			{
				sharedMesh = true;
				break;
			}
		}
		if (!sharedMesh)
			MeshLoader::Unload(objects[selectedObj].modelPath);
	}

	objects.erase(objects.begin() + selectedObj);
	selectedObj = -1;
}
