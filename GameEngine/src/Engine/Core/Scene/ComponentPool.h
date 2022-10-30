#pragma once
#include "Engine/Core/Core.h"
#include <typeinfo>

namespace Engine
{
	using ComponentType = uint64;
	class ComponentPool
	{
	public:
		ComponentPool(ComponentType typeID, uint64 m_ComponentSize);
		ComponentType GetTypeID() { return m_TypeID; }

		void* Allocate(uint64 entity);

	private:
		const ComponentType m_TypeID;
		const uint64 m_ComponentSize;

		uint32 m_Count;

		void* m_Components;

	};
}
