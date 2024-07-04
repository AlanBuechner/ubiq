#pragma once
#include "ShaderPass.h"

namespace Engine
{

	class ComputeShaderPass : public ShaderPass
	{
	protected:
		ComputeShaderPass(Ref<ShaderSorce> src, const std::string& passName);

	public:
		virtual ~ComputeShaderPass() {};
		static Ref<ComputeShaderPass> Create(Ref<ShaderSorce> src, const std::string& passName);

	protected:
		ComputePassConfig& m_PassConfig;
	};
}
