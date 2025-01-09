#include "Undo.h"

namespace Editor
{

	UndoManager::UndoManager(uint32 maxUndos)
	{
		m_Commands.Resize(maxUndos);
	}

	void UndoManager::PushCommand(Command* command)
	{
		delete m_Commands[m_Head];
		m_Commands[m_Head] = command;
		m_Head = (m_Head + 1) % m_Commands.Count();
	}

	void UndoManager::Undo()
	{
		m_Head = (m_Head - 1) % m_Commands.Count();
		Command* command = m_Commands[m_Head];
		if (command != nullptr)
			command->Undo();
	}

	void UndoManager::Clear()
	{
		for (uint32 i = 0; i < m_Commands.Count(); i++)
			delete m_Commands[i];
		m_Head = 0;
	}

}

