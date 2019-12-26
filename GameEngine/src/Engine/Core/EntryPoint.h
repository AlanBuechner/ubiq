#pragma once
#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	Engine::Instrumentor::Get().BeginSession("Startup", "startup.json");
	auto app = Engine::CreateApplication();
	Engine::Instrumentor::Get().EndSession();
	Engine::Instrumentor::Get().BeginSession("Runtime", "runtime.json");
	app->Run();
	Engine::Instrumentor::Get().EndSession();
	delete app;
}

#endif // PLATFORM_WINDOWS
