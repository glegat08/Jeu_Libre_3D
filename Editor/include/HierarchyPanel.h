#pragma once

#include "Scene/Scene.h"
#include "imgui.h"
#include <string>

namespace KGR
{
	namespace Editor
	{
		/// @brief Tree panel listing all entities in the scene, similar to Unreal's World Outliner.
		///
		/// Displays root entities and their children as a recursive tree.
		/// Left-click selects an entity. Right-click opens a context menu for add/delete.
		class HierarchyPanel
		{
		public:
			/// @param scene Pointer to the active scene.
			explicit HierarchyPanel(Scene* scene);

			/// @brief Draws the hierarchy panel. Call between BeginFrame/EndFrame.
			void Render();

			/// @brief Returns the currently selected entity.
			SceneEntity GetSelectedEntity() const { return m_selectedEntity; }

			/// @brief Programmatically selects an entity.
			void SetSelectedEntity(SceneEntity e) { m_selectedEntity = e; }

			/// @brief Updates the scene pointer (e.g. after play/stop swap).
			void SetScene(Scene* scene) { m_scene = scene; }

		private:
			/// @brief Recursively draws a tree node for the given entity and its children.
			void DrawEntityNode(SceneEntity e);

			Scene*      m_scene          = nullptr;
			SceneEntity m_selectedEntity = NullEntity;
			int         m_entityCounter  = 0;
		};
	}
}
