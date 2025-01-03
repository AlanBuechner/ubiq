#include "pch.h"
#include "UUID.h"

#include <random>
#include <unordered_map>


namespace Engine
{
	static std::random_device s_RandomDivice;
	static std::mt19937_64 s_Engine(s_RandomDivice());
	static std::uniform_int_distribution<uint64> s_UniformDistribution;

	UUID::UUID() : 
		m_UUID(s_UniformDistribution(s_Engine))
	{}

	UUID::UUID(uint64 uuid) :
		m_UUID(uuid)
	{}

}
