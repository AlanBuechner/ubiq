#pragma once
#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	auto app = Engine::CreateApplication();
	Engine::Instrumentor::Get().BeginSession("Runtime");
	app->Run();
	Engine::Instrumentor::Get().EndSession();
	delete app;
}

#endif // PLATFORM_WINDOWS
