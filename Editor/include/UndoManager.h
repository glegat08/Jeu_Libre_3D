#pragma once

#include <vector>
#include <memory>
#include "Commands/ICommand.h"
#include "Commands/ComponentCommand.h"
#include "Scene/Scene.h"

namespace KGR
{
	namespace Editor
	{
		/*
		* @brief manages the undo and redo history stacks
		*/
		class UndoManager
		{
		public:
			/*
			* @brief pushes a new command to the history and executes it
			* @param command is the command to execute and store
			*/
			void Push(std::unique_ptr<ICommand> command);

			/*
			* @brief undo the last action
			*/
			void Undo();

			/*
			* @brief redo the last undone action
			*/
			void Redo();

			/*
			* @brief clears both undo and redo histories
			*/
			void Clear();

			/*
			* @brief helper to quickly record a component modification
			* @tparam T is the component type
			* @param scene is the scene containing the entity
			* @param entity is the target entity
			* @param oldState is the state before modification
			* @param newState is the state after modification
			*/
			template<typename T>
			void RecordEdit(Scene* scene, SceneEntity entity, const T& oldState, const T& newState)
			{
				auto command = std::make_unique<EditComponentCommand<T>>(scene, entity, oldState, newState);
				Push(std::move(command));
			}

		private:
			std::vector<std::unique_ptr<ICommand>> m_undoStack;
			std::vector<std::unique_ptr<ICommand>> m_redoStack;
		};
	}
}