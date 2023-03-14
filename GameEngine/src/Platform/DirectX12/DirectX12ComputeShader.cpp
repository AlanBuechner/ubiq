#include "pch.h"
#include "DirectX12ComputeShader.h"
#include "DirectX12Context.h"
#include "Engine/Renderer/Renderer.h"
#include <fstream>

namespace Engine
{

	DirectX12ComputeShader::DirectX12ComputeShader(const std::string& code, const fs::path& file)
	{
		IDxcBlob* blob = nullptr;

		if (!code.empty())
		{
			ShaderBlobs cs = DirectX12ShaderCompiler::Get().Compile(code, file, ShaderType::Compute);
			if (!cs.object) return;
			blob = cs.object;
			ShaderSorce::SectionInfo info;
			DirectX12ShaderCompiler::Get().GetShaderParameters(cs, info, m_ReflectionData, ShaderType::Compute);
		}

		if (blob != nullptr)
		{
			m_Sig = DirectX12ShaderCompiler::Get().GenRootSignature(m_ReflectionData);
			for (auto& param : m_ReflectionData)
				m_UniformLocations[param.name] = param.rootIndex;

			GenPiplineState(blob);
		}
	}


	std::vector<Engine::ShaderParameter> DirectX12ComputeShader::GetReflectionData() const
	{
		return m_ReflectionData;
	}

	uint32 DirectX12ComputeShader::GetUniformLocation(const std::string& name) const
	{
		auto location = m_UniformLocations.find(name);
		if (location != m_UniformLocations.end())
			return location->second;
		return UINT32_MAX; // invalid location
	}

	void DirectX12ComputeShader::GenPiplineState(IDxcBlob* blob)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_Sig.Get();
		psoDesc.CS = { reinterpret_cast<UINT8*>(blob->GetBufferPointer()), blob->GetBufferSize() };
		context->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_State.GetAddressOf()));
	}

}

