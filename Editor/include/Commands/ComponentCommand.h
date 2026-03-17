#pragma once

#include "ICommand.h"
#include "Scene/Scene.h"

namespace KGR
{
	namespace Editor
	{
		/*
		* @brief comand to modify an existing ECS component
		* @tparam T the component type
		*/
		template<typename T>
		class EditComponentCommand : public ICommand
		{
		public:
			/*
			* @brief constructs an EditComponentCommand
			* @param scene is the scene containing the entity
			* @param entity is the target entity
			* @param oldState is the component state before modification
			* @param newState is the component state after modification
			*/
			EditComponentCommand(Scene* scene, SceneEntity entity, const T& oldState, const T& newState)
				: m_scene(scene), m_entity(entity), m_oldState(oldState), m_newState(newState) { }

			/*
			* @brief restores the old component state
			*/
			void Revert() override
			{
				m_scene->GetComponent<T>(m_entity) = m_oldState;
			}

			/*
			* @brief applies the new component state
			*/
			void Execute() override
			{
				m_scene->GetComponent<T>(m_entity) = m_newState;
			}

		private:
			Scene* m_scene;
			SceneEntity m_entity;
			T m_oldState;
			T m_newState;
		};
	}
}