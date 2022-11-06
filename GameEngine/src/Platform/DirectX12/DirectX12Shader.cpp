#include "pch.h"
#include "DirectX12Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include "Engine/Renderer/ShaderCompiler.h"

wrl::ComPtr<IDxcCompiler3> Engine::DirectX12Shader::s_Compiler;
wrl::ComPtr<IDxcUtils> Engine::DirectX12Shader::s_Utils;
wrl::ComPtr<IDxcIncludeHandler> Engine::DirectX12Shader::s_IncludeHandler;

Engine::ShaderPass::Uniform::Type GetFormatFromDesc(D3D12_SIGNATURE_PARAMETER_DESC& desc)
{
	if (desc.Mask == 1)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::ShaderPass::Uniform::Uint;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::ShaderPass::Uniform::Int;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::ShaderPass::Uniform::Float;
	}
	else if (desc.Mask <= 3)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::ShaderPass::Uniform::Uint2;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::ShaderPass::Uniform::Int2;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::ShaderPass::Uniform::Float2;
	}
	else if (desc.Mask <= 7)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::ShaderPass::Uniform::Uint3;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::ShaderPass::Uniform::Int3;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::ShaderPass::Uniform::Float3;
	}
	else if (desc.Mask <= 15)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::ShaderPass::Uniform::Uint4;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::ShaderPass::Uniform::Int4;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::ShaderPass::Uniform::Float4;
	}
	CORE_WARN("shader formate unknown defaulting to float");
	return Engine::ShaderPass::Uniform::Float;
}


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

	std::vector<ShaderPass::Uniform> DirectX12Shader::GetUniforms()
	{
		return std::vector<ShaderPass::Uniform>();
	}

	uint32 DirectX12Shader::GetUniformLocation(const std::string& name) const
	{
		auto& location = m_UniformLocations.find(name);
		if (location != m_UniformLocations.end())
			return location->second;
		return UINT32_MAX; // invalid location
	}

	void DirectX12Shader::Init()
	{
		if (!s_Compiler)
		{
			CORE_ASSERT_HRESULT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(s_Compiler.GetAddressOf())), "Failed to create compiler");
			CORE_ASSERT_HRESULT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(s_Utils.GetAddressOf())), "Failed To Create compiler utils");
			CORE_ASSERT_HRESULT(s_Utils->CreateDefaultIncludeHandler(s_IncludeHandler.GetAddressOf()), "Failed to Create include handler");
		}

		ByteCodeBlobs blobs{};

		const std::string& vsc = m_Src->m_Sections[m_PassConfig.vs];
		if (!vsc.empty())
		{
			ShaderBlobs vs = Compile(ShaderType::Vertex, vsc);
			if (!vs.object) return;
			blobs.vs = vs.object;
			Reflect(vs, ShaderType::Vertex);
		}

		const std::string& psc = m_Src->m_Sections[m_PassConfig.ps];
		if (!psc.empty())
		{
			ShaderBlobs ps = Compile(ShaderType::Pixel, psc);
			if (!ps.object) return;
			blobs.ps = ps.object;
			Reflect(ps, ShaderPass::Pixel);
		}

		if (blobs)
		{
			CreateRootSigniture();
			CreatePiplineState(blobs);
		}
	}

	DirectX12Shader::ShaderBlobs DirectX12Shader::Compile(ShaderType type, const std::string& code)
	{
		const std::wstring typeStrings[] = { L"vs_6_5", L"ps_6_5" };
		std::wstring profile = typeStrings[(int)log2((int)type)]; // take the log2 of the type enum to convert from bit mask to index

		LPCWSTR args[]
		{
			m_Src->file.c_str(),
			L"-E", L"main",
			L"-T", profile.c_str(),
			DXC_ARG_ALL_RESOURCES_BOUND,
#ifdef DEBUG
			DXC_ARG_SKIP_OPTIMIZATIONS,
#else
			DXC_ARG_OPTIMIZATION_LEVEL3,
#endif // DEBUG
			//L"-Qstrip_reflect", // strip reflection data to another blob
			DXC_ARG_DEBUG,
			L"-Qembed_debug", // strip debug data to another blob
		};

		DxcBuffer buffer;
		buffer.Encoding = DXC_CP_ACP;
		buffer.Ptr = code.c_str();
		buffer.Size = code.size();

		wrl::ComPtr<IDxcResult> result;
		if (FAILED(s_Compiler->Compile(&buffer, args, _countof(args), s_IncludeHandler.Get(), IID_PPV_ARGS(result.GetAddressOf()))))
		{
			CORE_ERROR("Failed to compile shader");
			return {};
		}

		wrl::ComPtr<IDxcBlobUtf8> errors;
		if(FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr)))
		{
			CORE_ERROR("Failed to compile shader");
			return {};
		}

		if (errors && errors->GetStringLength())
		{
			CORE_ERROR("Shader Compilation Errors & Warning: \n{0}", errors->GetStringPointer());
		}
		
		HRESULT status;
		CORE_ASSERT_HRESULT(result->GetStatus(&status), "Failed to get shader status");
		if (FAILED(status))
		{
			CORE_ERROR("Failed to compile shader \"{0}\"", m_Src->file.string());
			return {};
		}

		wrl::ComPtr<IDxcBlob> shader;
		result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(shader.GetAddressOf()), nullptr);
		wrl::ComPtr<IDxcBlob> reflection;
		result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflection.GetAddressOf()), nullptr);

		ShaderBlobs blobs;
		blobs.object = shader.Detach();
		blobs.reflection = reflection.Detach();
		return blobs;
	}

	void DirectX12Shader::Reflect(ShaderBlobs blobs, ShaderType type)
	{
		if(!blobs.reflection)
			return;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		wrl::ComPtr<ID3D12ShaderReflection > reflection;
		CORE_ASSERT_HRESULT(s_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		// bound resources
		m_ReflectionData.reserve(reflectionDesc.BoundResources);
		for (uint32 srvIndex = 0; srvIndex < reflectionDesc.BoundResources; srvIndex++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(srvIndex, &bindDesc);

			ShaderParameter data;
			data.shader = type;
			data.name = bindDesc.Name;
			data.reg = bindDesc.BindPoint;
			data.space = bindDesc.Space;
			data.count = bindDesc.BindCount;
			if (data.count == 0)
				data.count = -1;

			if (bindDesc.BindCount > 1 || bindDesc.Type == D3D_SIT_TEXTURE)
				data.type = ShaderParameter::PerameterType::DescriptorTable;
			else
			{
				if (bindDesc.Type == D3D_SIT_SAMPLER)
					data.type = ShaderParameter::PerameterType::StaticSampler;
				else
					data.type = ShaderParameter::PerameterType::Descriptor;
			}

			switch (bindDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				data.descType = ShaderParameter::DescriptorType::CBV; break;
			case D3D_SIT_TEXTURE:
				data.descType = ShaderParameter::DescriptorType::SRV; break;
			case D3D_SIT_SAMPLER:
				data.descType = ShaderParameter::DescriptorType::Sampler; break;
			default:
				break;
			}
			m_ReflectionData.push_back(data);
		}

		// input structure
		if (type == ShaderPass::Vertex)
		{
			m_InputElements.reserve(reflectionDesc.InputParameters);
			for (uint32 ipIndex = 0; ipIndex < reflectionDesc.InputParameters; ipIndex++)
			{
				D3D12_SIGNATURE_PARAMETER_DESC inputParam;
				reflection->GetInputParameterDesc(ipIndex, &inputParam);

				ShaderInputElement element;
				element.semanticName = inputParam.SemanticName;
				element.semanticIndex = inputParam.SemanticIndex;
				element.format = GetFormatFromDesc(inputParam);
				m_InputElements.push_back(element);
			}
		}
		else if(type == ShaderPass::Pixel)
		{
			m_RenderTargetFormates.reserve(reflectionDesc.OutputParameters);
			for (uint32 opIndex = 0; opIndex < reflectionDesc.OutputParameters; opIndex++)
			{
				D3D12_SIGNATURE_PARAMETER_DESC outputParam;
				reflection->GetOutputParameterDesc(opIndex, &outputParam);

				switch (outputParam.ComponentType)
				{
				case D3D_REGISTER_COMPONENT_FLOAT32:
					m_RenderTargetFormates.push_back(FrameBufferTextureFormat::RGBA8);
					break;
				case D3D_REGISTER_COMPONENT_UINT32:
					break;
				case D3D_REGISTER_COMPONENT_SINT32:
					m_RenderTargetFormates.push_back(FrameBufferTextureFormat::RED_INTEGER);
					break;


				default:
					break;
				}

			}
		}


	}

	D3D12_SHADER_VISIBILITY GetShaderVisibilityFlag(ShaderPass::ShaderType type)
	{
		switch (type)
		{
		case Engine::ShaderPass::Vertex:
			return D3D12_SHADER_VISIBILITY_VERTEX;
			break;
		case Engine::ShaderPass::Pixel:
			return D3D12_SHADER_VISIBILITY_PIXEL;
			break;
		default:
			return D3D12_SHADER_VISIBILITY_ALL;
			break;
		}
	}

	void DirectX12Shader::CreateRootSigniture()
	{
		// TODO: populate with reflection data
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsd;
		rsd.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		// create root parameters
		std::vector<D3D12_ROOT_PARAMETER1> params;
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

		uint32 numDescriptorTables = 0;
		for (uint32 i = 0; i < m_ReflectionData.size(); i++)
			if (m_ReflectionData[i].type == ShaderParameter::PerameterType::DescriptorTable) numDescriptorTables++;

		uint32 currentDescriptor = 0;
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges(numDescriptorTables);

		for (ShaderParameter& rd : m_ReflectionData)
		{
			if (rd.type != ShaderParameter::PerameterType::StaticSampler)
			{
				// populate root index
				rd.rootIndex = (uint32)params.size();
				m_UniformLocations[rd.name] = rd.rootIndex;

				// root parameters
				D3D12_ROOT_PARAMETER1 param;
				param.ShaderVisibility = GetShaderVisibilityFlag(rd.shader);
				if (rd.type == ShaderParameter::PerameterType::Constants)
				{
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
					param.Constants.Num32BitValues = rd.count;
					param.Constants.ShaderRegister = rd.reg;
					param.Constants.RegisterSpace = rd.space;
				}
				else if (rd.type == ShaderParameter::PerameterType::Descriptor)
				{
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					switch (rd.descType)
					{
					case ShaderParameter::DescriptorType::CBV:
						param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; break;
					case ShaderParameter::DescriptorType::SRV:
						param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; break;
					case ShaderParameter::DescriptorType::UAV:
						param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV; break;
					default:
						break;
					}
					param.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
					param.Descriptor.ShaderRegister = rd.reg;
					param.Descriptor.RegisterSpace = rd.space;
				}
				else if (rd.type == ShaderParameter::PerameterType::DescriptorTable)
				{
					D3D12_DESCRIPTOR_RANGE_TYPE type;
					switch (rd.descType)
					{
					case ShaderParameter::DescriptorType::CBV:
						type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					case ShaderParameter::DescriptorType::SRV:
						type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderParameter::DescriptorType::UAV:
						type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderParameter::DescriptorType::Sampler:
						type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
					default:
						break;
					}

					descriptorRanges[currentDescriptor].Init(type, rd.count, rd.reg, rd.space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					param.DescriptorTable.NumDescriptorRanges = 1;
					param.DescriptorTable.pDescriptorRanges = &descriptorRanges[currentDescriptor];
					currentDescriptor++;
				}

				params.push_back(param);
			}
			else
			{
				// static samplers
				D3D12_STATIC_SAMPLER_DESC ssd{};
				ssd.ShaderVisibility = GetShaderVisibilityFlag(rd.shader);
				ssd.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				ssd.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				ssd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				ssd.Filter = D3D12_FILTER_ANISOTROPIC;
				ssd.ShaderRegister = rd.reg;
				ssd.RegisterSpace = rd.space;
				samplers.push_back(ssd);
			}
		}

#if 0
		params.resize(3);
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		params[0].Constants.Num32BitValues = 2;
		params[0].Constants.ShaderRegister = 0;
		params[0].Constants.RegisterSpace = 0;

		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		params[1].Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
		params[1].Descriptor.ShaderRegister = 1;
		params[1].Descriptor.RegisterSpace = 0;

		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		params[2].DescriptorTable.NumDescriptorRanges = 1;
		D3D12_DESCRIPTOR_RANGE1 dr{};
		dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		dr.NumDescriptors = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		dr.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
		dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		dr.BaseShaderRegister = 0;
		dr.RegisterSpace = 0;
		params[2].DescriptorTable.pDescriptorRanges = &dr;

		// create static samplers
		D3D12_STATIC_SAMPLER_DESC ssd{};
		ssd.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		ssd.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		ssd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		ssd.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#endif 

		D3D12_ROOT_SIGNATURE_DESC1 desc{};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumParameters = (uint32)params.size();
		desc.pParameters = params.data();
		desc.NumStaticSamplers = (uint32)samplers.size();
		desc.pStaticSamplers = samplers.data();

		rsd.Desc_1_1 = desc;

		HRESULT hr = S_OK;
		ID3DBlob* rootSigBlob;
		ID3DBlob* errorBlob;
		hr = D3D12SerializeVersionedRootSignature(&rsd, &rootSigBlob, &errorBlob);
		if (FAILED(hr))
		{
			CORE_ERROR((const char*)errorBlob->GetBufferPointer());
			__debugbreak();
		}

		hr = context->GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(m_Sig.GetAddressOf()));
		CORE_ASSERT_HRESULT(hr, "Failed to create root signature");

		rootSigBlob->Release();
		if(errorBlob) errorBlob->Release();
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
