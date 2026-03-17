#pragma once

namespace KGR
{
	namespace Editor
	{
		/*
		* @brief base interface for all editor commands
		*/
		class ICommand
		{
		public:
			/*
			* @brief default virtual destructor
			*/
			virtual ~ICommand() = default;

			/*
			* @brief reverts the command's action
			*/
			virtual void Revert() = 0;

			/*
			* @brief executes or re-executes the command's action
			*/
			virtual void Execute() = 0;
		};
	}
}