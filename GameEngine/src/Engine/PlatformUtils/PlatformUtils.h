#pragma once
#include "Utils/Vector.h"
#include <string>

namespace Engine
{
	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		static std::string OpenFolder(std::string savedPath = "");

	};

	Utils::Vector<std::string> GetCommandLineArguments();

	bool GetEmbededResource(uint32 type, uint32 id, byte*& data, uint32& size);
	void UnloadEmbededResource(uint32 type, uint32 id);
}
