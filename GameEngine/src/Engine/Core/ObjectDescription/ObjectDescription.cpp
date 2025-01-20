#include "ObjectDescription.h"
#include "Engine/Core/Core.h"

namespace Engine
{
	std::unordered_map<uint64, ConverterBase*>& ConverterBase::GetObjectConverterFunctions()
	{
		static std::unordered_map<uint64, ConverterBase*> converterFunctions;
		return converterFunctions;
	}



	ObjectDescription::ObjectDescription(Type type) :
		m_Type(type)
	{}

	bool ObjectDescription::HasEntery(const std::string& key)
	{
		CORE_ASSERT(m_Type == ObjectDescription::Type::Object, "Description is not of type object");
		return m_Enteries.find(key) != m_Enteries.end();
	}

}

