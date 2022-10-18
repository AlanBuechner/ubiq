#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Engine
{
	class Flag
	{
	public:
		Flag();

		void Signal();

		void Clear();

		void Wait(bool waitVal = true);
	private:
		bool m_Flag;
		std::mutex m_Mutex;
		std::condition_variable m_CondVar;
	};
}
