#include "HierarchyPanel.h"

namespace KGR
{
    namespace Editor
    {
        HierarchyPanel::HierarchyPanel(Scene* scene)
            : m_scene(scene)
        {
        }

        void HierarchyPanel::Render(Picker& picker)
        {
            ImGui::Begin("Hierarchy");

            if (m_scene)
            {
                if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                    ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("Add Entity"))
                    {
                        std::string name = "Entity_" + std::to_string(m_entityCounter++);
                        SceneEntity created = m_scene->CreateEntity(name);
                        // a new entity created from the menu becomes the sole selection
                        picker.Select(created, false);
                    }
                    ImGui::EndPopup();
                }

                for (SceneEntity root : m_scene->GetRootEntities())
                    DrawEntityNode(root, picker);
            }
            else
            {
                ImGui::TextDisabled("No scene loaded");
            }

            // click on empty space clears the selection
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
                picker.ClearSelection();

            ImGui::End();
        }

        void HierarchyPanel::DrawEntityNode(SceneEntity e, Picker& picker)
        {
            if (!m_scene)
                return;

            SceneRegistry& reg = m_scene->GetRegistry();

            std::string label = "Entity";
            if (reg.HasComponent<NameComponent>(e))
                label = reg.GetComponent<NameComponent>(e).name;

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

            if (picker.isSelected(e))
                flags |= ImGuiTreeNodeFlags_Selected;

            bool hasChildren = reg.HasComponent<HierarchyComponent>(e)
                && !reg.GetComponent<HierarchyComponent>(e).m_children.empty();

            if (!hasChildren)
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

            ImGui::PushID(static_cast<int>(e));
            bool opened = ImGui::TreeNodeEx(label.c_str(), flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                // Ctrl+click adds to the selection, plain click replaces it
                bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                picker.Select(e, ctrl);
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Add Child"))
                {
                    std::string childName = "Entity_" + std::to_string(m_entityCounter++);
                    SceneEntity child = m_scene->CreateEntity(childName, e);
                    picker.Select(child, false);
                }

                if (ImGui::MenuItem("Delete"))
                {
                    // clear selection first so Picker does not hold a stale handle
                    picker.Deselect(e);
                    m_scene->DestroyEntity(e);

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
                for (SceneEntity child : reg.GetComponent<HierarchyComponent>(e).m_children)
                    DrawEntityNode(child, picker);
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }
}