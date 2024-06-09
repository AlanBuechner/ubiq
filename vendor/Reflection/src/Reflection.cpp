#include "Reflection.h"

Reflect::Registry* Reflect::Registry::s_Instance;

namespace Reflect {

	void Registry::AddClass(const Class& c)
	{
		// check that the class was not already registered
		if (m_ClassesByName.find(c.GetSname()) != m_ClassesByName.end())
			return;
		m_Classes.push_back(c);
		Class* registerdClass = &m_Classes.back();
		m_ClassesByName[c.GetSname()] = registerdClass;
		m_ClassesByTypeID[c.GetTypeID()] = registerdClass;
		if (!c.GetGroup().empty())
			m_ClassGroups[c.GetGroup()].Push(registerdClass);
		for(Property& prop : registerdClass->properties)
			prop.c = registerdClass;
		for (Function& func : registerdClass->functions)
			func.c = registerdClass;
	}

	Registry* Registry::GetRegistry() {
		if (s_Instance)
			return s_Instance;

		return s_Instance = new Registry;
	}

} // namespace Reflect
