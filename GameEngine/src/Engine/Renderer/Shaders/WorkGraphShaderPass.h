#pragma once
#include "ShaderPass.h"

namespace Engine
{

	class WorkGraphShaderPass : public ShaderPass
	{
	protected:
		WorkGraphShaderPass(Ref<ShaderSorce> src, const std::string& passName);

	public:
		virtual ~WorkGraphShaderPass() {};
		static Ref<WorkGraphShaderPass> Create(Ref<ShaderSorce> src, const std::string& passName);

	protected:
		WorkGraphPassConfig& m_PassConfig;
	};
}
