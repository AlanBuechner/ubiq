#include "pch.h"
#include "DirectX12GraphicsShaderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Shaders/ShaderCompiler.h"


DXGI_FORMAT GetDXGIFormatFromShaderUniformType(Engine::UniformType type)
{
	switch (type)
	{
	case Engine::UniformType::Uint:		return DXGI_FORMAT_R32_UINT;
	case Engine::UniformType::Int:		return DXGI_FORMAT_R32_SINT;
	case Engine::UniformType::Float:	return DXGI_FORMAT_R32_FLOAT;
	case Engine::UniformType::Uint2:	return DXGI_FORMAT_R32G32_UINT;
	case Engine::UniformType::Int2:		return DXGI_FORMAT_R32G32_SINT;
	case Engine::UniformType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
	case Engine::UniformType::Uint3:	return DXGI_FORMAT_R32G32B32_UINT;
	case Engine::UniformType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
	case Engine::UniformType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
	case Engine::UniformType::Uint4:	return DXGI_FORMAT_R32G32B32A32_UINT;
	case Engine::UniformType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
	case Engine::UniformType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}


namespace Engine
{

	DirectX12GraphicsShaderPass::DirectX12GraphicsShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		GraphicsShaderPass(src, passName)
	{
		Init();
	}

	DirectX12GraphicsShaderPass::~DirectX12GraphicsShaderPass()
	{
		m_Sig->Release();
	}

	ID3D12PipelineState* DirectX12GraphicsShaderPass::GetPipelineState(Ref<FrameBuffer> target)
	{
		std::vector<TextureFormat> formates;

		TextureFormat depthFormat = TextureFormat::None;
		for (Ref<RenderTarget2D> rt : target->GetAttachments())
		{
			TextureFormat format = rt->GetResource()->GetFormat();
			if (IsTextureFormatDepthStencil(format))
				depthFormat = format;
			else
				formates.push_back(format);
		}
		formates.push_back(depthFormat);

		auto state = m_PiplineStates.find(formates);
		if (state == m_PiplineStates.end())
		{
			ID3D12PipelineState* pipline = CreatePiplineState(formates);
			m_PiplineStates[formates] = pipline;
			return pipline;
		}

		return state->second;
	}

	void DirectX12GraphicsShaderPass::Init()
	{


		{ // Vertex Shader
			SectionInfo vsi = m_Src->m_Sections[m_PassConfig.vs];
			const std::string& vsc = vsi.m_SectionCode.str();
			if (!vsc.empty())
			{
				ShaderBlobs vs = DirectX12ShaderCompiler::Get().Compile(vsc, m_Src->file, ShaderType::Vertex);
				if (!vs.object) return;
				m_Blobs.vs = vs.object;
				DirectX12ShaderCompiler::Get().GetShaderParameters(vs, vsi, m_ReflectionData, ShaderType::Vertex);
				DirectX12ShaderCompiler::Get().GetInputLayout(vs, m_InputElements);
			}
		}

		{ // Pixel Shader
			SectionInfo psi = m_Src->m_Sections[m_PassConfig.ps];
			const std::string& psc = psi.m_SectionCode.str();
			if (!psc.empty())
			{
				ShaderBlobs ps = DirectX12ShaderCompiler::Get().Compile(psc, m_Src->file, ShaderType::Pixel);
				if (!ps.object) return;
				m_Blobs.ps = ps.object;
				DirectX12ShaderCompiler::Get().GetShaderParameters(ps, psi, m_ReflectionData, ShaderType::Pixel);
			}
		}

		if (m_Blobs)
		{
			// create root signature
			m_Sig = DirectX12ShaderCompiler::Get().GenRootSignature(m_ReflectionData);
			CORE_ASSERT(m_Sig, "Faild To Create Root Signature");

			for (auto& param : m_ReflectionData)
				m_UniformLocations[param.name] = param.rootIndex;
		}
	}

	ID3D12PipelineState* DirectX12GraphicsShaderPass::CreatePiplineState(const std::vector<TextureFormat>& formats)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		ID3D12PipelineState* state;

		CORE_ASSERT(m_Sig, "Root Signature is null for some reason");

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
		desc.pRootSignature = m_Sig; // set root signature
		desc.InputLayout = { ies.data(), (UINT)ies.size() }; // set input layout

		// set shader byte code
		desc.VS = { m_Blobs.vs->GetBufferPointer(), m_Blobs.vs->GetBufferSize() };
		desc.PS = { m_Blobs.ps->GetBufferPointer(), m_Blobs.ps->GetBufferSize() };

		// set topology
		switch (m_PassConfig.topology)
		{
		case Topology::Triangle:	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		case Topology::Line:		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
		case Topology::Point:		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
		default:					desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		}

		// set cull mode
		switch (m_PassConfig.cullMode)
		{
		case CullMode::Back:	desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
		case CullMode::Front:	desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; break;
		case CullMode::None:	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
		default:				desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
		}

		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.FrontCounterClockwise = FALSE;
		desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		desc.RasterizerState.DepthClipEnable = TRUE;
		desc.RasterizerState.MultisampleEnable = FALSE;
		desc.RasterizerState.AntialiasedLineEnable = FALSE;
		desc.RasterizerState.ForcedSampleCount = 0;
		desc.RasterizerState.ConservativeRaster = m_PassConfig.enableConservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		desc.BlendState.AlphaToCoverageEnable = FALSE;
		desc.BlendState.IndependentBlendEnable = FALSE;

		desc.NumRenderTargets = (uint32)formats.size();
		for (uint32 i = 0; i < formats.size(); i++)
		{
			desc.RTVFormats[i] = GetDXGITextureFormat(formats[i]);

			switch (formats[i])
			{
			case TextureFormat::R8_UINT:
			case TextureFormat::RG8_UINT:
			case TextureFormat::RGBA8_UINT:

			case TextureFormat::R8_SINT:
			case TextureFormat::RG8_SINT:
			case TextureFormat::RGBA8_SINT:

			case TextureFormat::R16_UINT:
			case TextureFormat::RG16_UINT:
			case TextureFormat::RGBA16_UINT:

			case TextureFormat::R16_SINT:
			case TextureFormat::RG16_SINT:
			case TextureFormat::RGBA16_SINT:

			case TextureFormat::R32_UINT:
			case TextureFormat::RG32_UINT:
			case TextureFormat::RGBA32_UINT:

			case TextureFormat::R32_SINT:
			case TextureFormat::RG32_SINT:
			case TextureFormat::RGBA32_SINT:
			{
				desc.BlendState.RenderTarget[i].BlendEnable = FALSE;
				desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
				desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				desc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
				desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
				desc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
				desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			default:
			{
				desc.BlendState.RenderTarget[i].BlendEnable = m_PassConfig.blendMode != BlendMode::None;
				desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
				desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				switch (m_PassConfig.blendMode)
				{
				case BlendMode::Blend:		desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; break;
				case BlendMode::Add:		desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_SRC_ALPHA; break;
				default:					desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ONE; break;
				}

				desc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
				desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
				desc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
				desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			}
		}

		if (formats.back() != TextureFormat::None)
		{
			desc.DepthStencilState.DepthEnable = TRUE;
			desc.DSVFormat = GetDXGITextureFormat(formats.back());
			desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

			switch (m_PassConfig.depthTest)
			{
			case DepthTest::Less:			desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; break;
			case DepthTest::LessOrEqual:	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; break;
			case DepthTest::Greater:		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER; break;
			case DepthTest::GreaterOrEqual:	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; break;
			case DepthTest::None:			desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; break;
			}

			desc.DepthStencilState.StencilEnable = TRUE;
			desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
			desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
			const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
			{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
			desc.DepthStencilState.FrontFace = defaultStencilOp;
			desc.DepthStencilState.BackFace = defaultStencilOp;
		}

		desc.SampleMask = UINT_MAX;
		desc.SampleDesc.Count = 1;

		context->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&state));

		return state;
	}

}
