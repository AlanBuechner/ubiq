#include "pch.h"
#include "DirectX12Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include "Engine/Renderer/ShaderCompiler.h"


DXGI_FORMAT GetDXGIFormatFromShaderUniformType(Engine::ShaderPass::Uniform::Type type)
{
	switch (type)
	{
	case Engine::ShaderPass::Uniform::Uint:		return DXGI_FORMAT_R32_UINT;
	case Engine::ShaderPass::Uniform::Int:		return DXGI_FORMAT_R32_SINT;
	case Engine::ShaderPass::Uniform::Float:	return DXGI_FORMAT_R32_FLOAT;
	case Engine::ShaderPass::Uniform::Uint2:	return DXGI_FORMAT_R32G32_UINT;
	case Engine::ShaderPass::Uniform::Int2:		return DXGI_FORMAT_R32G32_SINT;
	case Engine::ShaderPass::Uniform::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
	case Engine::ShaderPass::Uniform::Uint3:	return DXGI_FORMAT_R32G32B32_UINT;
	case Engine::ShaderPass::Uniform::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
	case Engine::ShaderPass::Uniform::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
	case Engine::ShaderPass::Uniform::Uint4:	return DXGI_FORMAT_R32G32B32A32_UINT;
	case Engine::ShaderPass::Uniform::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
	case Engine::ShaderPass::Uniform::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	return DXGI_FORMAT_UNKNOWN;
}


namespace Engine
{

	DirectX12Shader::DirectX12Shader(Ref<ShaderSorce> src, const std::string& passName) :
		m_Src(src), m_PassName(passName), m_PassConfig(src->config.FindPass(passName))
	{
		Init();
	}

	DirectX12Shader::~DirectX12Shader()
	{
		//if(m_Sig)
		//	m_Sig->Release();
	}

	std::vector<Engine::ShaderParameter> DirectX12Shader::GetReflectionData() const
	{
		return m_ReflectionData;
	}

	uint32 DirectX12Shader::GetUniformLocation(const std::string& name) const
	{
		auto location = m_UniformLocations.find(name);
		if (location != m_UniformLocations.end())
			return location->second;
		return UINT32_MAX; // invalid location
	}

	void DirectX12Shader::Init()
	{
		ByteCodeBlobs blobs{};

		const std::string& vsc = m_Src->m_Sections[m_PassConfig.vs];
		if (!vsc.empty())
		{
			ShaderBlobs vs = DirectX12ShaderCompiler::Get().Compile(vsc, m_Src->file, ShaderType::Vertex);
			if (!vs.object) return;
			blobs.vs = vs.object;
			DirectX12ShaderCompiler::Get().GetShaderParameters(vs, m_ReflectionData, ShaderType::Vertex);
			DirectX12ShaderCompiler::Get().GetInputLayout(vs, m_InputElements);
		}

		const std::string& psc = m_Src->m_Sections[m_PassConfig.ps];
		if (!psc.empty())
		{
			ShaderBlobs ps = DirectX12ShaderCompiler::Get().Compile(psc, m_Src->file, ShaderType::Pixel);
			if (!ps.object) return;
			blobs.ps = ps.object;
			DirectX12ShaderCompiler::Get().GetShaderParameters(ps, m_ReflectionData, ShaderType::Pixel);
			DirectX12ShaderCompiler::Get().GetOutputLayout(ps, m_RenderTargetFormates);
		}

		if (blobs)
		{
			// create root signature
			m_Sig = DirectX12ShaderCompiler::Get().GenRootSignature(m_ReflectionData);
			for (auto& param : m_ReflectionData)
				m_UniformLocations[param.name] = param.rootIndex;

			// create pipeline state object
			CreatePiplineState(blobs);
		}
	}

	void DirectX12Shader::CreatePiplineState(ByteCodeBlobs blobs)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		std::vector<D3D12_INPUT_ELEMENT_DESC> ies(m_InputElements.size());
		for (uint32 i = 0; i < m_InputElements.size(); i++)
		{
			ShaderInputElement& ie = m_InputElements[i];
			uint32 slot = 0;
			uint32 stepRate = 0;
			D3D12_INPUT_CLASSIFICATION classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			if (ie.semanticName.rfind("I_", 0) == 0)
			{
				slot = 1;
				stepRate = 1;
				classification = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			}

			ies[i] = { ie.semanticName.c_str(), ie.semanticIndex, GetDXGIFormatFromShaderUniformType(ie.format), slot, D3D12_APPEND_ALIGNED_ELEMENT, classification, stepRate };
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = m_Sig.Get();
		desc.InputLayout = { ies.data(), (UINT)ies.size() };
		switch (m_Src->config.topology)
		{
		case ShaderConfig::Triangle:
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		case ShaderConfig::Line:
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
		case ShaderConfig::Point:
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
		}
		desc.VS = { blobs.vs->GetBufferPointer(), blobs.vs->GetBufferSize() };
		desc.PS = { blobs.ps->GetBufferPointer(), blobs.ps->GetBufferSize() };
		desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		desc.RasterizerState.FrontCounterClockwise = FALSE;
		desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		desc.RasterizerState.DepthClipEnable = TRUE;
		desc.RasterizerState.MultisampleEnable = FALSE;
		desc.RasterizerState.AntialiasedLineEnable = FALSE;
		desc.RasterizerState.ForcedSampleCount = 0;
		desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		desc.BlendState.AlphaToCoverageEnable = FALSE;
		desc.BlendState.IndependentBlendEnable = FALSE;
		desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
		desc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		desc.DepthStencilState.DepthEnable = TRUE;
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.DepthStencilState.StencilEnable = TRUE;
		desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
		{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		desc.DepthStencilState.FrontFace = defaultStencilOp;
		desc.DepthStencilState.BackFace = defaultStencilOp;

		desc.NumRenderTargets = (uint32)m_RenderTargetFormates.size();
		for(uint32 i = 0; i < m_RenderTargetFormates.size(); i++)
			desc.RTVFormats[i] = UbiqToDXGI(m_RenderTargetFormates[i]);
		desc.SampleMask = UINT_MAX;
		desc.SampleDesc.Count = 1;

		context->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_State.GetAddressOf()));
	}

}
