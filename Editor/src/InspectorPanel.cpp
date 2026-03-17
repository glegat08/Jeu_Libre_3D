#include "InspectorPanel.h"
#include "Context.h"

namespace KGR
{
    namespace Editor
    {
        InspectorPanel::InspectorPanel(Context* context, Scene* scene)
            : m_context(context), m_scene(scene)
        {
        }

        void InspectorPanel::Render(SceneEntity selected)
        {
            ImGui::Begin("Inspector");

            if (!m_scene || selected == NullEntity)
            {
                ImGui::TextDisabled("No entity selected");
                ImGui::End();
                return;
            }

            DrawNameComponent(selected);
            ImGui::Separator();
            DrawTransformComponent(selected);
            ImGui::Separator();
            DrawRegisteredComponents(selected);

            ImGui::End();
        }

        void InspectorPanel::DrawNameComponent(SceneEntity e)
        {
            SceneRegistry& reg = m_scene->GetRegistry();

            if (!reg.HasComponent<NameComponent>(e))
                return;

            auto& nameComp = reg.GetComponent<NameComponent>(e);

            char buffer[256] = {};
            strncpy_s(buffer, nameComp.name.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("Name", buffer, sizeof(buffer)) && buffer[0] != '\0')
                nameComp.name = buffer;
        }

        void InspectorPanel::DrawTransformComponent(SceneEntity e)
        {
            SceneRegistry& reg = m_scene->GetRegistry();

            if (!reg.HasComponent<TransformComponent>(e))
                return;

            auto& transform = reg.GetComponent<TransformComponent>(e);
            static TransformComponent initialState;

            auto checkUndoRedo = [&]()
                {
                    if (ImGui::IsItemActivated())
                        initialState = transform;

                    if (ImGui::IsItemDeactivatedAfterEdit())
                        m_context->GetUndoManager().RecordEdit(m_scene, e, initialState, transform);
                };

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                glm::vec3 position = transform.GetPosition();
                if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f))
                    transform.SetPosition(position);                   
                checkUndoRedo();

                glm::vec3 rotationDeg = glm::degrees(transform.GetRotation());
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotationDeg), 0.5f))
                    transform.SetRotation(glm::radians(rotationDeg));
                checkUndoRedo();

                glm::vec3 scale = transform.GetScale();
                if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.05f, 0.01f, 100.0f))
                    transform.SetScale(scale * 2.0f);
                checkUndoRedo();
            }
        }

        void InspectorPanel::DrawRegisteredComponents(SceneEntity e)
        {
            if (!m_scene)
                return;

            SceneRegistry& reg = m_scene->GetRegistry();

            for (const auto& [label, drawFn] : m_scene->GetInspectorRegistry())
            {
                if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    drawFn(e, reg);
            }
        }
    }
}