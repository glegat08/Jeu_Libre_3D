#pragma once

#include "Scene/Scene.h"
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace KGR
{
	namespace Editor
	{
		/// @brief Properties panel for the selected entity, similar to Unreal's Details panel.
		///
		/// Displays and edits the Name, Transform, and any user-registered components
		/// via Scene::GetInspectorRegistry().
		class InspectorPanel
		{
		public:
			/// @param scene Pointer to the active scene.
			explicit InspectorPanel(Scene* scene);

			/// @brief Draws the inspector for the given entity.
			/// @param selected The entity to inspect. Pass NullEntity to show nothing.
			void Render(SceneEntity selected);

			/// @brief Updates the scene pointer (e.g. after play/stop swap).
			void SetScene(Scene* scene) { m_scene = scene; }

		private:
			/// @brief Draws the NameComponent editor.
			void DrawNameComponent(SceneEntity e);

			/// @brief Draws the TransformComponent editor (position, rotation, scale).
			void DrawTransformComponent(SceneEntity e);

			/// @brief Draws all user-registered inspector functions.
			void DrawRegisteredComponents(SceneEntity e);

			Scene* m_scene = nullptr;
		};
	}
}
