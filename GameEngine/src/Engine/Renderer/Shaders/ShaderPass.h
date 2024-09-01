#pragma once
#include "Engine/Core/Core.h"
#include "ShaderCompiler.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
	class ShaderPass
	{
	protected:
		ShaderPass(Ref<ShaderSorce> src, const std::string& passName);

	public:
		virtual ~ShaderPass() {};
		inline Utils::Vector<ShaderParameter> GetReflectionData() { return m_ReflectionData; };
		uint32 GetUniformLocation(const std::string& name);
		bool IsComputeShader() { return m_ComputeShader; }

	protected:
		std::unordered_map<std::string, uint32> m_UniformLocations;
		Utils::Vector<ShaderParameter> m_ReflectionData;
		bool m_ComputeShader;

		Ref<ShaderSorce> m_Src;
		std::string m_PassName;
	};
}
