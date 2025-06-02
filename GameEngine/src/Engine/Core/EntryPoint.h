#pragma once
#include "Engine/Core/Core.h"
#include <iostream>
#include <memory>

#ifdef PLATFORM_WINDOWS

extern Engine::Application* CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	Profiler::Instrumentor::Get().RecordData(true);
	Profiler::Instrumentor::Get().RegisterThread("Main", 0);
	Engine::Application* app = CreateApplication();

	app->Run();
	delete app;
}

#endif // PLATFORM_WINDOWS
