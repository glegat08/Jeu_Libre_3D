#pragma once

#include "Scene/Scene.h"
#include "imgui.h"
#include <memory>

namespace KGR
{
    namespace Editor
    {
        /** @brief play / stop toolbar */
        class Toolbar
        {
        public:
            /** @param scene pointer to the editor scene */
            explicit Toolbar(Scene* scene);

            /** @brief draws the toolbar, call between BeginFrame/EndFrame */
            void Render();

            /**
             * @return the active scene (runtime clone in play mode, editor scene otherwise)
             */
            Scene* GetActiveScene() const;

            /** @return true if play mode is currently active */
            bool IsPlaying() const { return m_isPlaying; }

        private:
            Scene* m_editorScene = nullptr;
            std::unique_ptr<Scene> m_runtimeScene = nullptr;
            bool m_isPlaying = false;
        };
    }
}