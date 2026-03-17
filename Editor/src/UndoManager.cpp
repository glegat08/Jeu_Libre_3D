#include "UndoManager.h"

namespace KGR
{
	namespace Editor
	{
		void UndoManager::Push(std::unique_ptr<ICommand> command)
		{
			command->Execute();
			m_undoStack.push_back(std::move(command));
			m_redoStack.clear();
		}

		void UndoManager::Undo()
		{
			if (m_undoStack.empty())
				return;

			auto command = std::move(m_undoStack.back());
			m_undoStack.pop_back();
			command->Revert();
			m_redoStack.push_back(std::move(command));
		}

		void UndoManager::Redo()
		{
			if (m_redoStack.empty())
				return;

			auto command = std::move(m_redoStack.back());
			m_redoStack.pop_back();
			command->Execute();
			m_undoStack.push_back(std::move(command));
		}

		void UndoManager::Clear()
		{
			m_undoStack.clear();
			m_redoStack.clear();
		}
	}
}