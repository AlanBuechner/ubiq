#pragma once
#include "ReflectionData.h"
#include <filesystem>

namespace fs = std::filesystem;

void WriteCode(const fs::path& path, const std::string& projectName, const ReflectionData& data);
