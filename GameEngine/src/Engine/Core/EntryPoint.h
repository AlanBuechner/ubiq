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

	Engine::Instrumentor::Get().RecordData(true);
	Engine::Instrumentor::Get().BeginSession("Startup", "startup.json");
	Engine::Instrumentor::Get().RegisterThread("Main", 0);
	auto app = CreateApplication();
	Engine::Instrumentor::Get().EndSession();
	Engine::Instrumentor::Get().BeginSession("Runtime", "runtime.json");
	app->Run();
	Engine::Instrumentor::Get().EndSession();
	delete app;
}

#endif // PLATFORM_WINDOWS
