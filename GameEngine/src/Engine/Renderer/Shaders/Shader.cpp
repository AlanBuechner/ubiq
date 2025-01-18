#include "pch.h"
#include "Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Shaders/ShaderCompiler.h"
#include "GraphicsShaderPass.h"
#include "ComputeShaderPass.h"

#include "Engine/PlatformUtils/PlatformUtils.h"
#include "EngineResource.h"

#include <fstream>

namespace Engine
{


	Shader::Shader(const std::string& code, const fs::path& file)
	{
		std::stringstream is(code);
		Ref<ShaderSorce> src = ShaderCompiler::LoadFromSrc(is, file);

		for (const GraphicsPassConfig& pass : src->config.graphicsPasses)
			m_GraphicsPasses[pass.passName] = GraphicsShaderPass::Create(src, pass.passName);
		for (const ComputePassConfig& pass : src->config.computePasses)
			m_ComputePasses[pass.passName] = ComputeShaderPass::Create(src, pass.passName);
		for (const WorkGraphPassConfig& pass : src->config.workGraphPasses)
			m_WorkGraphPasses[pass.passName] = WorkGraphShaderPass::Create(src, pass.passName);

		m_Params = src->config.params;
	}

	Ref<GraphicsShaderPass> Shader::GetGraphicsPass(const std::string& passName)
	{
		auto pass = m_GraphicsPasses.find(passName);
		if (pass == m_GraphicsPasses.end())
			return nullptr;
		return pass->second;
	}

	Ref<ComputeShaderPass> Shader::GetComputePass(const std::string& passName)
	{
		auto pass = m_ComputePasses.find(passName);
		if (pass == m_ComputePasses.end())
			return nullptr;
		return pass->second;
	}

	Ref<WorkGraphShaderPass> Shader::GetWorkGraphPass(const std::string& passName)
	{
		auto pass = m_WorkGraphPasses.find(passName);
		if (pass == m_WorkGraphPasses.end())
			return nullptr;
		return pass->second;
	}

	Utils::Vector<MaterialParameter>& Shader::GetParams()
	{
		return m_Params;
	}

	Ref<Shader> Shader::Create(const fs::path& file)
	{
		std::ifstream ifs(file);
		if (ifs.fail())
		{
			CORE_ERROR("Cant open file {0}", file.string());
			return nullptr;
		}

		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		return CreateFromSrc(str, file);
	}

	Ref<Shader> Shader::CreateFromSrc(const std::string& src, const fs::path& file /*= ""*/)
	{
		return CreateRef<Shader>(src, file);
	}

}
