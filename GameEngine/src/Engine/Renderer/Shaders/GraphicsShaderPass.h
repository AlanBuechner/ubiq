#pragma once
#include "ShaderPass.h"

namespace Engine
{
	class GraphicsShaderPass : public ShaderPass
	{
	protected:
		GraphicsShaderPass(Ref<ShaderSorce> src, const std::string& passName);

	public:
		virtual ~GraphicsShaderPass() {};
		static Ref<GraphicsShaderPass> Create(Ref<ShaderSorce> src, const std::string& passName);

	protected:
		GraphicsPassConfig& m_PassConfig;
	};
}
