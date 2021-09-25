#pragma once
#include <xhash>


namespace Engine
{

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
}


namespace std
{
	template<>
	struct hash<Engine::UUID>
	{
		std::size_t operator()(const Engine::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
