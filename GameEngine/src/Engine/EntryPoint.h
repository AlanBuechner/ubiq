#pragma once

#ifdef PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	auto app = Engine::CreateApplication();
	app->Run();
	delete app;
}

#endif // PLATFORM_WINDOWS
