#pragma once

#include "Scene/Scene.h"
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace KGR
{
    namespace Editor
    {
        class Context;

        /**
         * @brief properties panel for the selected entity
         *
         * draws and edits Name, Transform, and any component registered via Scene::RegisterInspector()
         */
        class InspectorPanel
        {
        public:
            /** @param scene pointer to the active scene */
            explicit InspectorPanel(Context* context, Scene* scene);

            /**
             * @brief draws the inspector for the given entity
             * @param selected entity to inspect, pass NullEntity to show nothing
             */
            void Render(SceneEntity selected);

            /** @brief swaps the scene pointer, call this after play/stop */
            void SetScene(Scene* scene) { m_scene = scene; }

        private:
            void DrawNameComponent(SceneEntity e);
            void DrawTransformComponent(SceneEntity e);
            void DrawRegisteredComponents(SceneEntity e);

            Scene* m_scene = nullptr;
            Context* m_context;
        };
    }
}