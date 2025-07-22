#pragma once
#include "Flag.h"
#include <unordered_map>

namespace Engine
{
	typedef void (*JobFunc)(void*);

	class NamedJobThread
	{
	public:
		NamedJobThread(const std::string& name);

		void SetData(void* data = nullptr) { if (IsExecuting()) CORE_WARN("Setting data while thread is in flight \"{0}\"", m_Name); m_Data = data; }
		void Invoke() { m_Flag.WaitAndSignal(); }
		void Invoke(void* data) { Wait(); SetData(data); m_Flag.WaitAndSignal(); }
		void Wait() { m_Flag.Wait(false); }
		bool IsExecuting() { return m_Flag.GetFlag(); }
		void SetFunc(JobFunc func) { m_Func = func; }

	private:
		void RunInternal();

	private:
		void* m_Data = nullptr;
		std::string m_Name;
		std::thread m_Thread;
		Flag m_Flag;
		JobFunc m_Func;
	};


	class JobSystem
	{
	public:

		static NamedJobThread* AddNamedJob(const std::string& name, JobFunc func);
		static NamedJobThread* GetNamedJob(const std::string& name);
		static void InvokeNamedThread(const std::string& name);
		static void WaitForNamedThread(const std::string& name);

	private:
		static std::unordered_map<std::string, NamedJobThread*> s_NamedJobs;

	};
}
