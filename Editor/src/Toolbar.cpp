#include "Toolbar.h"

namespace KGR
{
    namespace Editor
    {
        Toolbar::Toolbar(Scene* scene)
            : m_editorScene(scene)
        {
        }

        void Toolbar::Render()
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoScrollWithMouse;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
            ImGui::Begin("##Toolbar", nullptr, flags);

            float buttonWidth = 80.0f;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth - ImGui::GetStyle().WindowPadding.x) * 0.5f);

            if (!m_isPlaying)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.4f, 0.05f, 1.0f));

                if (ImGui::Button("Play", ImVec2(buttonWidth, 0)) && m_editorScene)
                {
                    m_runtimeScene = m_editorScene->Clone();
                    m_isPlaying = true;
                }

                ImGui::PopStyleColor(3);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));

                if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
                {
                    m_runtimeScene.reset();
                    m_isPlaying = false;
                }

                ImGui::PopStyleColor(3);
            }

            ImGui::End();
            ImGui::PopStyleVar();
        }

        Scene* Toolbar::GetActiveScene() const
        {
            if (m_isPlaying && m_runtimeScene)
                return m_runtimeScene.get();

            return m_editorScene;
        }
    }
}