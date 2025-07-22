#include "JsonObjectDescription.h"

namespace Engine
{
	ObjectDescription LoadObjectDescriptionFromJson(const nlohmann::json& json)
	{
		if (json.is_object())
		{
			ObjectDescription desc(ObjectDescription::Type::Object);

			// load each entry
			for (auto it = json.begin(); it != json.end(); it++)
				desc[it.key()] = LoadObjectDescriptionFromJson(it.value());

			return desc;
		}
		else if (json.is_array())
		{
			ObjectDescription desc(ObjectDescription::Type::Array);

			// load each entry
			for (auto value : json)
				desc.GetAsObjectArray().Push(LoadObjectDescriptionFromJson(value));

			return desc;
		}
		else if (json.is_string())
			return ObjectDescription::CreateFrom(json.get<std::string>());
		else if (json.is_number_float())
			return ObjectDescription::CreateFrom(json.get<double>());
		else if (json.is_number_integer())
		{
			if (json.is_number_unsigned())
				return ObjectDescription::CreateFrom(json.get<uint64>());
			else
				return ObjectDescription::CreateFrom(json.get<int64>());
		}
		else if (json.is_boolean())
			return ObjectDescription::CreateFrom(json.get<bool>());
		CORE_ASSERT(false, "Could not load json");
	}

	ObjectDescription LoadObjectDescriptionFromJsonString(const std::string& str)
	{
		nlohmann::json json(str);
		return LoadObjectDescriptionFromJson(json);
	}

	ObjectDescription LoadObjectDescriptionFromJsonFile(const fs::path& file)
	{
		std::ifstream f(file);
		nlohmann::json json;
		f >> json;
		return LoadObjectDescriptionFromJson(json);
	}
}
