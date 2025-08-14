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

		Math::UVector3 GetThreadGroupSize() { return m_ThreadGroupSize; }

	protected:
		ComputePassConfig& m_PassConfig;
		Math::UVector3 m_ThreadGroupSize = { 0,0,0 };
	};
}
