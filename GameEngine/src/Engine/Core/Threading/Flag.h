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

		void SetFlag(bool flag);
		void Signal() { SetFlag(true); }
		void Clear() { SetFlag(false); }
		void Wait(bool waitVal = true);
		void WaitAndSignal() { Wait(false); Signal(); }

		bool GetFlag() { return m_Flag; }

	private:
		bool m_Flag = false;
		std::mutex m_Mutex;
		std::condition_variable m_CondVar;
	};
}
