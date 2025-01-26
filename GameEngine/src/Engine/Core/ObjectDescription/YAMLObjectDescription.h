#pragma once
#include "ObjectDescription.h"
#include "Engine/Core/Core.h"
#include <yaml-cpp/yaml.h>

namespace Engine
{
	ObjectDescription LoadObjectDescriptionFromYAML(const YAML::Node& yaml);
	ObjectDescription LoadObjectDescriptionFromYAMLString(const std::string& str);
	ObjectDescription LoadObjectDescriptionFromYAMLFile(const fs::path& file);

	void WriteObjectDescriptionToYAML(const ObjectDescription& desc, YAML::Emitter& out);
}
