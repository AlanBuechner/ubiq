#include "JobSystem.h"
#include "Engine/Core/Core.h"

std::unordered_map<std::string, Engine::NamedJobThread*> Engine::JobSystem::s_NamedJobs;

namespace Engine
{

	NamedJobThread::NamedJobThread(const std::string& name) :
		m_Name(name)
	{
		m_Thread = std::thread(std::bind(&NamedJobThread::RunInternal, this));
	}

	void NamedJobThread::RunInternal()
	{
		while (true)
		{
			m_Flag.Wait(true); // wait to be signaled
			if(m_Func != nullptr)
				m_Func(m_Data);
			m_Flag.Clear(); // reset flag
		}
	}

	NamedJobThread* JobSystem::AddNamedJob(const std::string& name, JobFunc func)
	{
		NamedJobThread* thread = new NamedJobThread(name);
		thread->SetFunc(func);
		s_NamedJobs[name] = thread;
		return thread;
	}

	NamedJobThread* JobSystem::GetNamedJob(const std::string& name)
	{
		auto loc = s_NamedJobs.find(name);
		if (loc == s_NamedJobs.end())
			return nullptr;
		return loc->second;
	}

	void JobSystem::InvokeNamedThread(const std::string& name)
	{
		NamedJobThread* thread = GetNamedJob(name);
		if (thread != nullptr)
			thread->Invoke();
	}

	void JobSystem::WaitForNamedThread(const std::string& name)
	{
		NamedJobThread* thread = GetNamedJob(name);
		if (thread != nullptr)
			thread->Wait();
	}

}

