#pragma once
#include <xhash>
#include "Core.h"
#include "Platform/Windows/Win.h"

namespace Engine
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64 uuid);
		UUID(const UUID&) = default;

		operator uint64() const { return m_UUID; }
#if defined(PLATFORM_WINDOWS)
		operator LUID() const { return *(LUID*)(void*)&m_UUID; }
#endif

	private:
		uint64 m_UUID;
	};
}


namespace std
{
	template<>
	struct hash<Engine::UUID>
	{
		std::size_t operator()(const Engine::UUID& uuid) const
		{
			return hash<uint64>()((uint64)uuid);
		}
	};
}
