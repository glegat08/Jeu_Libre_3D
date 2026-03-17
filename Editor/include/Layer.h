#pragma once

#include "imgui.h"

namespace KGR
{
    namespace Editor
    {
        /** @brief manages the ImGui dockspace and the editor visual style */
        class Layer
        {
        public:
            /**
             * @brief applies the KGR dark theme to ImGui
             * @note call once right after ImGui::CreateContext(), before the first frame
             */
            static void ApplyStyle();

            /**
             * @brief begins the fullscreen dockspace host window
             * @note call at the very beginning of each frame, before any panel Render()
             */
            static void BeginDockspace();

            /**
             * @brief ends the dockspace host window
             * @note call after all panels have been rendered
             */
            static void EndDockspace();

        private:
            /** @brief splits the dockspace into Toolbar, Hierarchy, Viewport and Inspector */
            static void BuildDefaultLayout(ImGuiID dockspaceId);
            static bool m_layoutBuilt;
        };
    }
}