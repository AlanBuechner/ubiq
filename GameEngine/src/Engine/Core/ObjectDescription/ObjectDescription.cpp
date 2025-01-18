#include "ObjectDescription.h"

namespace Engine
{

	ObjectDescription::ObjectDescription(Type type) :
		m_Type(type)
	{}

	bool ObjectDescription::IsNumber(bool* isFloat) const
	{
		if (m_Type != Type::String)
			return false;

		std::string::const_iterator it = m_String.begin();
		int dotCount = 0;
		while (it != m_String.end())
		{
			if (!std::isdigit(*it))
			{
				bool isDot = *it == '.';
				dotCount += isDot;
				if (!isDot || dotCount > 1) break;
			}
			++it;
		}
		bool isNumber = !m_String.empty() && it == m_String.end();
		if (isNumber && isFloat != nullptr)
			*isFloat = dotCount == 1;
		return isNumber;
	}

	std::string* ObjectDescription::TryGetAsString()
	{
		if (m_Type != Type::String)
			return nullptr;
		return &m_String;
	}

	Utils::Vector<Engine::ObjectDescription>* ObjectDescription::TryGetAsObjectArray()
	{
		if (m_Type != Type::Array)
			return nullptr;
		return &m_Vector;
	}

	std::unordered_map<std::string, Engine::ObjectDescription>* ObjectDescription::TryGetAsDescriptionMap()
	{
		if (m_Type != Type::Object)
			return nullptr;
		return &m_Enteries;
	}



}

