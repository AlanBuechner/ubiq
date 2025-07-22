#include "YAMLObjectDescription.h"

namespace Engine
{

	ObjectDescription LoadObjectDescriptionFromYAML(const YAML::Node& yaml)
	{
		if (yaml.IsMap())
		{
			ObjectDescription desc(ObjectDescription::Type::Object);
			for (YAML::const_iterator it = yaml.begin(); it != yaml.end(); ++it)
				desc.GetAsDescriptionMap()[it->first.as<std::string>()] = LoadObjectDescriptionFromYAML(it->second);
			return desc;
		}
		else if (yaml.IsSequence())
		{
			ObjectDescription desc(ObjectDescription::Type::Array);
			for (auto& val : yaml)
				desc.GetAsObjectArray().Push(LoadObjectDescriptionFromYAML(val));
			return desc;
		}
		else if (yaml.IsScalar())
		{
#define TRY_DECODE(type)\
			if (type val; YAML::convert<type>::decode(yaml, val))\
				return ObjectDescription::CreateFrom(val)

			TRY_DECODE(bool);
			else TRY_DECODE(uint64);
			else TRY_DECODE(int64);
			else TRY_DECODE(double);
			else TRY_DECODE(std::string);

#undef TRY_DECODE
		}
		CORE_ASSERT(false, "Could not load yaml");
	}

	ObjectDescription LoadObjectDescriptionFromYAMLString(const std::string& str)
	{
		YAML::Node data = YAML::Load(str);
		return LoadObjectDescriptionFromYAML(data);
	}

	ObjectDescription LoadObjectDescriptionFromYAMLFile(const fs::path& file)
	{
		std::ifstream stream(file);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		return LoadObjectDescriptionFromYAMLString(strStream.str());
	}

	void WriteObjectDescriptionToYAML(const ObjectDescription& desc, YAML::Emitter& out)
	{
		if (desc.IsObject())
		{
			out << YAML::BeginMap;
			const std::unordered_map<std::string, ObjectDescription>& map = desc.GetAsDescriptionMap();
			for (auto [Key, value] : map)
			{
				out << YAML::Key << Key << YAML::Value;
				WriteObjectDescriptionToYAML(value, out);
			}
			out << YAML::EndMap;
		}
		else if (desc.IsArray())
		{
			const Utils::Vector<ObjectDescription>& vec = desc.GetAsObjectArray();
			if (!vec.Empty() && !(vec[0].IsObject() || vec[0].IsArray()))
				out << YAML::Flow;
			out << YAML::BeginSeq;
			for (uint32 i = 0; i < vec.Count(); i++)
				WriteObjectDescriptionToYAML(vec[i], out);
			out << YAML::EndSeq;
		}
		else if (desc.IsString())	out << desc.GetAsString();
		else if (desc.IsSint())		out << desc.GetAsInt();
		else if (desc.IsUint())		out << desc.GetAsUint();
		else if (desc.IsFloat())	out << desc.GetAsFloat();
		else if (desc.IsBool())		out << desc.GetAsBool();
	}

}

