#include "ShaderPass.h"

namespace Engine
{

	ShaderPass::ShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		m_Src(src), m_PassName(passName)
	{}

	uint32 ShaderPass::GetUniformLocation(const std::string& name)
	{
		auto location = m_UniformLocations.find(name);
		if (location != m_UniformLocations.end())
			return location->second;
		return UINT32_MAX; // invalid location
	}

}

