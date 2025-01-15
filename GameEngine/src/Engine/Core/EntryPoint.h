#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Util/Performance.h"
#include <iostream>
#include <memory>

#ifdef PLATFORM_WINDOWS

extern Engine::Application* CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	Profiler::Instrumentor::Get().RecordData(true);
	Profiler::Instrumentor::Get().BeginSession("Startup", "startup.json");
	Profiler::Instrumentor::Get().RegisterThread("Main", 0);
	Engine::Application* app = CreateApplication();
	Profiler::Instrumentor::Get().EndSession();
	Profiler::Instrumentor::Get().BeginSession("Runtime", "runtime.json");
	app->Run();
	Profiler::Instrumentor::Get().EndSession();
	delete app;
}

#endif // PLATFORM_WINDOWS
