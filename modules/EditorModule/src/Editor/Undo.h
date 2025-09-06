#pragma once
#include "Utils/Vector.h"

namespace Editor
{
	class Command
	{
	public:
		virtual void Undo() = 0;
	};


	class UndoManager
	{
	public:
		UndoManager(uint32 maxUndos);

		void PushCommand(Command* command);
		template<typename T, typename... Args>
		void PushCommand(const Args&... args)
		{
			PushCommand(new T(std::forward(args)...));
		}

		void Undo();
		void Clear();

	private:

		uint32 m_Head = 0;
		Utils::Vector<Command*> m_Commands;
	};






	// commands
	class FuncCommand : public Command
	{
	public:
		using Func = std::function<void()>;
		FuncCommand(Func func) : m_Func(func) {}

	private:
		Func m_Func;
	};

}
