#include "Time.h"
#include <chrono>

double Utils::Time::s_DeltaTime = 0.0f;
double Utils::Time::s_LastFrameTime = Utils::Time::GetTime();

namespace Utils
{
	using namespace std::chrono;

	double Time::GetTime()
	{
		return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
	}

	void Time::UpdateDeltaTime()
	{
		double time = GetTime();
		s_DeltaTime = time - s_LastFrameTime;
		s_LastFrameTime = time;
	}
}
