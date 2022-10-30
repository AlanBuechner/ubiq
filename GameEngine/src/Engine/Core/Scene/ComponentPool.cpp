#include "pch.h"
#include "ComponentPool.h"

namespace Engine
{


	ComponentPool::ComponentPool(ComponentType typeID, uint64 componentSize) :
		m_TypeID(typeID), m_ComponentSize(componentSize)
	{}

	void* ComponentPool::Allocate(uint64 entity)
	{
		return malloc(m_ComponentSize);
	}

}

