#include "DirectX12ComputeShaderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Shaders/ShaderCompiler.h"

namespace Engine
{
	
	DirectX12ComputeShaderPass::DirectX12ComputeShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		ComputeShaderPass(src, passName)
	{
		Init();
	}

	DirectX12ComputeShaderPass::~DirectX12ComputeShaderPass()
	{
		m_Sig->Release();
		m_Pipline->Release();
	}

	void DirectX12ComputeShaderPass::Init()
	{
		SectionInfo csi = m_Src->m_Sections[m_PassConfig.cs];
		const std::string& csc = csi.m_SectionCode.str();
		if (!csc.empty())
		{
			ShaderBlobs cs = DirectX12ShaderCompiler::Get().Compile(csc, m_Src->file, ShaderType::Compute);
			if (!cs.object) return;
			m_Blobs.cs = cs.object;
			DirectX12ShaderCompiler::Get().GetShaderParameters(cs, csi, m_ReflectionData, ShaderType::Compute);
		}

		if (m_Blobs)
		{
			// create root signature
			m_Sig = DirectX12ShaderCompiler::Get().GenRootSignature(m_ReflectionData);
			CORE_ASSERT(m_Sig, "Failed To Create Root Signature");
			m_Pipline = CreatePiplineState();
			CORE_ASSERT(m_Pipline, "Failed to Create Pipline State Object");

			for (auto& param : m_ReflectionData)
				m_UniformLocations[param.name] = param.rootIndex;
		}
	}

	ID3D12PipelineState* DirectX12ComputeShaderPass::CreatePiplineState()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		ID3D12PipelineState* state;

		CORE_ASSERT(m_Sig, "Root Signature is null for some reason")

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_Sig;
		psoDesc.CS = { m_Blobs.cs->GetBufferPointer(), m_Blobs.cs->GetBufferSize() };
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&state)),
			"Faild to create compute pipline state");

		return state;
	}

}

