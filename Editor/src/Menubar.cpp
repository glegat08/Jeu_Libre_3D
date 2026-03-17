#include "MenuBar.h"
#include "KGR_ImGui.h"

namespace KGR
{
    namespace Editor
    {
        MenuBar::MenuBar(SaveCallback onSave, LoadCallback onLoad)
            : m_onSave(std::move(onSave)), m_onLoad(std::move(onLoad))
        {
        }

        void MenuBar::Render()
        {
            if (!ImGui::BeginMainMenuBar())
                return;

            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open..."))
                {
                    std::string path = KGR::_ImGui::ImGuiCore::OpenFile(
                        "KGR Scene (*.kscene)\0*.kscene\0All Files\0*.*\0");

                    if (!path.empty())
                        m_onLoad(path);
                }

                if (ImGui::MenuItem("Save As..."))
                {
                    std::string path = KGR::_ImGui::ImGuiCore::SaveFile(
                        "KGR Scene (*.kscene)\0*.kscene\0All Files\0*.*\0");

                    if (!path.empty())
                    {
                        std::filesystem::path p(path);
                        if (p.extension() != ".kscene")
                            p.replace_extension(".kscene");

                        m_onSave(p);
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
}