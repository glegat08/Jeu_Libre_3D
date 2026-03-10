#include "HierarchyPanel.h"

namespace KGR
{
	namespace Editor
	{
		HierarchyPanel::HierarchyPanel(Scene* scene)
			: m_scene(scene)
		{
		}

		void HierarchyPanel::Render()
		{
			ImGui::Begin("Hierarchy");

			if (m_scene)
			{
				if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
				{
					if (ImGui::MenuItem("Add Entity"))
					{
						std::string name = "Entity_" + std::to_string(m_entityCounter++);
						SceneEntity created = m_scene->CreateEntity(name);
						m_selectedEntity = created;
					}
					ImGui::EndPopup();
				}

				std::vector<SceneEntity> roots = m_scene->GetRootEntities();
				for (SceneEntity root : roots)
					DrawEntityNode(root);
			}
			else
			{
				ImGui::TextDisabled("No scene loaded");
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				if (!ImGui::IsAnyItemHovered())
					m_selectedEntity = NullEntity;
			}

			ImGui::End();
		}

        void HierarchyPanel::DrawEntityNode(SceneEntity e)
        {
            if (!m_scene)
                return;

            SceneRegistry& reg = m_scene->GetRegistry();

            std::string label = "Entity";
            if (reg.HasComponent<NameComponent>(e))
                label = reg.GetComponent<NameComponent>(e).name;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

            if (m_selectedEntity == e)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool hasChildren = false;
            if (reg.HasComponent<HierarchyComponent>(e))
                hasChildren = !reg.GetComponent<HierarchyComponent>(e).m_children.empty();

            if (!hasChildren)
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

            ImGui::PushID(static_cast<int>(e));
            bool opened = ImGui::TreeNodeEx(label.c_str(), flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                m_selectedEntity = e;

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Add Child"))
                {
                    std::string childName = "Entity_" + std::to_string(m_entityCounter++);
                    SceneEntity child = m_scene->CreateEntity(childName, e);
                    m_selectedEntity = child;
                }

                if (ImGui::MenuItem("Delete"))
                {
                    m_scene->DestroyEntity(e);
                    if (m_selectedEntity == e)
                        m_selectedEntity = NullEntity;

                    ImGui::EndPopup();
                    ImGui::PopID();

                    if (opened && hasChildren)
                        ImGui::TreePop();

                    return;
                }

                ImGui::EndPopup();
            }

            if (opened && hasChildren)
            {
                auto& hierarchy = reg.GetComponent<HierarchyComponent>(e);
                for (SceneEntity child : hierarchy.m_children)
                    DrawEntityNode(child);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }
}
