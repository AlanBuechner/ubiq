#pragma once
#include "ObjectDescription.h"
#include "Engine/Core/Core.h"
#include <nlohmann/json.hpp>

namespace Engine
{
	ObjectDescription LoadObjectDescriptionFromJson(const nlohmann::json& json);
	ObjectDescription LoadObjectDescriptionFromJsonString(const std::string& json);
	ObjectDescription LoadObjectDescriptionFromJsonFile(const fs::path& file);
}

