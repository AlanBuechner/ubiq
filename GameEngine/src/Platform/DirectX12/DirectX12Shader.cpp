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
		m_Sig->Release();
	}

	std::vector<ShaderParameter> DirectX12Shader::GetReflectionData() const
	{
		return m_ReflectionData;
	}

	wrl::ComPtr<ID3D12PipelineState> DirectX12Shader::GetPipelineState(Ref<FrameBuffer> target)
	{
		std::vector<TextureFormat> formates;

		if (!m_ComputeShader)
		{
			TextureFormat depthFormat = TextureFormat::None;
			for (Ref<RenderTarget2D> rt : target->GetAttachments())
			{
				TextureFormat format = rt->GetResource()->GetFormat();
				if (IsDepthStencil(format))
					depthFormat = format;
				else
					formates.push_back(format);
			}
			formates.push_back(depthFormat);
		}

		auto state = m_PiplineStates.find(formates);
		if (state == m_PiplineStates.end())
			return CreatePiplineState(formates);

		return state->second;
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
		{ // Compute Shader
			ShaderSorce::SectionInfo csi = m_Src->m_Sections[m_PassConfig.cs];
			const std::string& csc = csi.m_SectionCode.str();
			if (!csc.empty())
			{
				ShaderBlobs cs = DirectX12ShaderCompiler::Get().Compile(csc, m_Src->file, ShaderType::Compute);
				if (!cs.object) return;
				m_Blobs.cs = cs.object;
				DirectX12ShaderCompiler::Get().GetShaderParameters(cs, csi, m_ReflectionData, ShaderType::Compute);
				m_ComputeShader = true;
				//DirectX12ShaderCompiler::Get().GetOutputLayout(cs, m_RenderTargetFormates);
			}
		}

		if (!m_ComputeShader)
		{
			{ // Vertex Shader
				ShaderSorce::SectionInfo vsi = m_Src->m_Sections[m_PassConfig.vs];
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
				ShaderSorce::SectionInfo psi = m_Src->m_Sections[m_PassConfig.ps];
				const std::string& psc = psi.m_SectionCode.str();
				if (!psc.empty())
				{
					ShaderBlobs ps = DirectX12ShaderCompiler::Get().Compile(psc, m_Src->file, ShaderType::Pixel);
					if (!ps.object) return;
					m_Blobs.ps = ps.object;
					DirectX12ShaderCompiler::Get().GetShaderParameters(ps, psi, m_ReflectionData, ShaderType::Pixel);
					DirectX12ShaderCompiler::Get().GetOutputLayout(ps, m_RenderTargetFormates);
				}
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

	wrl::ComPtr<ID3D12PipelineState> DirectX12Shader::CreatePiplineState(const std::vector<TextureFormat>& formats)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		wrl::ComPtr<ID3D12PipelineState> state;

		CORE_ASSERT(m_Sig, "Root Signature is null for some reason")

		if (m_ComputeShader)
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_Sig;
			psoDesc.CS = { m_Blobs.cs->GetBufferPointer(), m_Blobs.cs->GetBufferSize() };
			CORE_ASSERT_HRESULT(context->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(state.GetAddressOf())), 
				"Faild to create compute pipline state");
		}
		else
		{


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
			desc.pRootSignature = m_Sig;
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
			desc.VS = { m_Blobs.vs->GetBufferPointer(), m_Blobs.vs->GetBufferSize() };
			desc.PS = { m_Blobs.ps->GetBufferPointer(), m_Blobs.ps->GetBufferSize() };

			desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			if (m_PassConfig.cullMode == ShaderConfig::RenderPass::CullMode::Back)
				desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			else if (m_PassConfig.cullMode == ShaderConfig::RenderPass::CullMode::Front)
				desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
			else if (m_PassConfig.cullMode == ShaderConfig::RenderPass::CullMode::None)
				desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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
					desc.BlendState.RenderTarget[i].BlendEnable = m_PassConfig.blendMode != ShaderConfig::RenderPass::BlendMode::None;
					desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
					desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
					if (m_PassConfig.blendMode == ShaderConfig::RenderPass::BlendMode::Blend)
						desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
					else if (m_PassConfig.blendMode == ShaderConfig::RenderPass::BlendMode::Add)
						desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_SRC_ALPHA;
					else
						desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
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
				if (m_PassConfig.depthTest == ShaderConfig::RenderPass::DepthTest::Less)
					desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				else if (m_PassConfig.depthTest == ShaderConfig::RenderPass::DepthTest::LessOrEqual)
					desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
				else if (m_PassConfig.depthTest == ShaderConfig::RenderPass::DepthTest::Greater)
					desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
				else if (m_PassConfig.depthTest == ShaderConfig::RenderPass::DepthTest::GreaterOrEqual)
					desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
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

			context->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(state.GetAddressOf()));
		}

		m_PiplineStates[formats] = state;
		return state;
	}

}
