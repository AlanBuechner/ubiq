#include "pch.h"
#include "Flag.h"

namespace Engine
{

	Engine::Flag::Flag() :
		m_Flag{ false }
	{}

	void Engine::Flag::Signal()
	{
		std::lock_guard g(m_Mutex);
		m_Flag = true;
		m_CondVar.notify_all();
	}

	void Engine::Flag::Clear()
	{
		std::lock_guard g(m_Mutex);
		m_Flag = false;
		m_CondVar.notify_all();
	}

	void Engine::Flag::Wait(bool waitVal)
	{
		std::unique_lock lock(m_Mutex);
		m_CondVar.wait(lock, [this, waitVal]() { 
			return m_Flag == waitVal; 
		});
	}

}
