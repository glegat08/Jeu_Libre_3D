#include "Layer.h"
#include "imgui_internal.h"

namespace KGR
{
    namespace Editor
    {
        bool Layer::m_layoutBuilt = false;

        void Layer::ApplyStyle()
        {
            ImGuiStyle& style = ImGui::GetStyle();

            style.WindowPadding = ImVec2(8.0f, 6.0f);
            style.FramePadding = ImVec2(6.0f, 3.0f);
            style.ItemSpacing = ImVec2(8.0f, 4.0f);
            style.IndentSpacing = 18.0f;
            style.ScrollbarSize = 12.0f;
            style.GrabMinSize = 10.0f;

            style.WindowRounding = 0.0f;
            style.FrameRounding = 2.0f;
            style.ScrollbarRounding = 2.0f;
            style.GrabRounding = 2.0f;
            style.TabRounding = 2.0f;
            style.PopupRounding = 2.0f;

            // No borders on docked windows (cleaner look)
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;

            // Thin separator between docked panels
            style.DockingSeparatorSize = 2.0f;

            ImVec4* c = style.Colors;

            c[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
            c[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
            c[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);

            // Title bars — slightly lighter than the window
            c[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.2f);
            c[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 0.2f);
            c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 0.2f);

            // Tabs
            c[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
            c[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
            c[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            c[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
            c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
            c[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
            c[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

            // Frames (input fields, collapsing headers, etc.)
            c[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            c[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
            c[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);

            // Header (CollapsingHeader, TreeNode when selected)
            c[ImGuiCol_Header] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
            c[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
            c[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

            // Buttons
            c[ImGuiCol_Button] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
            c[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
            c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

            // Accent colour for sliders, checkboxes, drag fields
            c[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.45f, 0.10f, 1.00f);
            c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.60f, 0.15f, 1.00f);
            c[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.55f, 0.10f, 1.00f);

            // Separators and borders
            c[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
            c[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            c[ImGuiCol_SeparatorActive] = ImVec4(0.80f, 0.45f, 0.10f, 1.00f);

            // Text
            c[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
            c[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

            // Scrollbar
            c[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
            c[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
            c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

            // Selection highlight
            c[ImGuiCol_NavHighlight] = ImVec4(0.80f, 0.45f, 0.10f, 1.00f);
        }

        void Layer::BeginDockspace()
        {
            // Mirror the main OS window exactly so the dockspace fills everything.
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);

            // The dockspace host window must be invisible and non-interactive.
            ImGuiWindowFlags hostFlags =
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("##DockspaceHost", nullptr, hostFlags);
            ImGui::PopStyleVar(3);

            ImGuiID dockspaceId = ImGui::GetID("KGRDockspace");

            // Build the default split layout only once.
            // After that, ImGui persists it in imgui.ini automatically.
            if (!m_layoutBuilt)
            {
                BuildDefaultLayout(dockspaceId);
                m_layoutBuilt = true;
            }

            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f),
                ImGuiDockNodeFlags_PassthruCentralNode);
        }

        void Layer::EndDockspace()
        {
            ImGui::End();
        }

        void Layer::BuildDefaultLayout(ImGuiID dockspaceId)
        {
            // Clear any previously saved layout to start fresh.
            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId,
                ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);

            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::DockBuilderSetNodeSize(dockspaceId, vp->WorkSize);

            ImGuiID idToolbar, idRest;
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Up, 0.04f, &idToolbar, &idRest);

            ImGuiID idHierarchy, idMain;
            ImGui::DockBuilderSplitNode(idRest, ImGuiDir_Left, 0.20f, &idHierarchy, &idMain);

            ImGuiID idViewport, idInspector;
            ImGui::DockBuilderSplitNode(idMain, ImGuiDir_Right, 0.28f, &idInspector, &idViewport);

            ImGui::DockBuilderDockWindow("##Toolbar", idToolbar);
            ImGui::DockBuilderDockWindow("Hierarchy", idHierarchy);
            ImGui::DockBuilderDockWindow("Inspector", idInspector);
            ImGui::DockBuilderDockWindow("Viewport", idViewport);

            ImGui::DockBuilderFinish(dockspaceId);
        }
    }
}
