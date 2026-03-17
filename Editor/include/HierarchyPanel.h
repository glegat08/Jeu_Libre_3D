#pragma once

#include "Scene/Scene.h"
#include "Picker.h"
#include "imgui.h"
#include <string>

namespace KGR
{
    namespace Editor
    {
        /**
         * @brief tree panel listing all entities in the scene
         *
         * left-click selects an entity, right-click opens a context menu to add or delete
         */
        class HierarchyPanel
        {
        public:
            /** @param scene pointer to the active scene */
            explicit HierarchyPanel(Scene* scene);

            /**
             * @brief draws the hierarchy panel
             * @param picker current selection state, updated on click
             */
            void Render(Picker& picker);

            /** @brief swaps the scene pointer, call this after play/stop */
            void SetScene(Scene* scene) { m_scene = scene; }

        private:
            /** @brief recursively draws one entity and its children as a tree node */
            void DrawEntityNode(SceneEntity e, Picker& picker);

            Scene* m_scene = nullptr;
            int m_entityCounter = 0;
        };
    }
}