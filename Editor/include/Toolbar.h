#pragma once

#include "Scene/Scene.h"
#include "imgui.h"
#include <memory>

namespace KGR
{
	namespace Editor
	{
		/// @brief Play / Stop toolbar
		///
		/// On Play, the editor scene is cloned. The runtime uses the clone.
		/// On Stop, the clone is discarded and the original scene is restored.
		class Toolbar
		{
		public:
			/// @param scene Pointer to the editor (original) scene.
			explicit Toolbar(Scene* scene);

			/// @brief Draws the toolbar. Call between BeginFrame/EndFrame.
			void Render();

			/// @brief Returns the scene that should be used for rendering/logic.
			///        During play mode this is the clone; otherwise the editor scene.
			Scene* GetActiveScene() const;

			/// @brief Returns true if play mode is currently active.
			bool IsPlaying() const { return m_isPlaying; }

			/// @brief Updates the editor scene pointer.
			void SetScene(Scene* scene) { m_editorScene = scene; }

		private:
			Scene*                  m_editorScene  = nullptr;
			std::unique_ptr<Scene>  m_runtimeScene = nullptr;
			bool                    m_isPlaying    = false;
		};
	}
}
