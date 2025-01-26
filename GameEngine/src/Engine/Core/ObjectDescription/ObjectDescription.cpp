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

	ObjectDescription& ObjectDescription::SetType(Type type)
	{
		m_Type = type;
		return *this;
	}

	bool ObjectDescription::HasEntery(const std::string& key) const
	{
		CORE_ASSERT(m_Type == ObjectDescription::Type::Object, "Description is not of type object");
		return m_Enteries.find(key) != m_Enteries.end();
	}

}



ADD_OBJECT_CONVERTER(std::string);

ADD_OBJECT_CONVERTER(uint8);
ADD_OBJECT_CONVERTER(uint16);
ADD_OBJECT_CONVERTER(uint32);
ADD_OBJECT_CONVERTER(uint64);

ADD_OBJECT_CONVERTER(int8);
ADD_OBJECT_CONVERTER(int16);
ADD_OBJECT_CONVERTER(int32);
ADD_OBJECT_CONVERTER(int64);

ADD_OBJECT_CONVERTER(float);
ADD_OBJECT_CONVERTER(double);

ADD_OBJECT_CONVERTER(Math::Vector2);
ADD_OBJECT_CONVERTER(Math::Vector3);
ADD_OBJECT_CONVERTER(Math::Vector4);

ADD_OBJECT_CONVERTER(bool);

