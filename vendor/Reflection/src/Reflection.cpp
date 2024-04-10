#include "pch.h"
#include "Reflection.h"

Reflect::Registry* Reflect::Registry::s_Instance;

namespace Reflect {
	const Class& Registry::GetClass(const std::string& name) { return m_Classes[name]; }
	void Registry::AddClass(const Class& c) 
	{
		const Class& registerdClass = (m_Classes[c.GetSname()] = c);
		if(!c.GetGroup().empty())
			m_ClassGroups[c.GetGroup()].push_back(&registerdClass);
	}
	Registry* Registry::GetRegistry() {
		if (s_Instance)
			return s_Instance;

		return s_Instance = new Registry;
	}

} // namespace Reflect
