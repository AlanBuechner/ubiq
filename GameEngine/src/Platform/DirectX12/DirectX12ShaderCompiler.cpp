#include "pch.h"
#include "DirectX12ShaderCompiler.h"
#include "DirectX12Context.h"
#include "Engine/Renderer/Renderer.h"


Engine::DirectX12ShaderCompiler* Engine::DirectX12ShaderCompiler::s_Instance;


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

D3D12_SHADER_VISIBILITY GetShaderVisibilityFlag(Engine::ShaderType type)
{
	switch (type)
	{
	case Engine::ShaderType::Vertex:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case Engine::ShaderType::Pixel:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	case Engine::ShaderType::Compute:
	default:
		return D3D12_SHADER_VISIBILITY_ALL;
		break;
	}
}

namespace Engine
{

	DirectX12ShaderCompiler::DirectX12ShaderCompiler()
	{
		CORE_ASSERT_HRESULT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.GetAddressOf())), "Failed to create compiler");
		CORE_ASSERT_HRESULT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_Utils.GetAddressOf())), "Failed To Create compiler utils");
		CORE_ASSERT_HRESULT(m_Utils->CreateDefaultIncludeHandler(m_IncludeHandler.GetAddressOf()), "Failed to Create include handler");
	}

	DirectX12ShaderCompiler& DirectX12ShaderCompiler::Get()
	{
		if (!s_Instance)
			s_Instance = new DirectX12ShaderCompiler();

		return *s_Instance;
	}

	ShaderBlobs DirectX12ShaderCompiler::Compile(const std::string& code, const fs::path& file, ShaderType type)
	{
		const std::wstring typeStrings[] = { L"vs_6_5", L"ps_6_5", L"cs_6_5" };
		std::wstring profile = typeStrings[(int)log2((int)type)]; // take the log2 of the type enum to convert from bit mask to index

		LPCWSTR args[]
		{
			file.c_str(),
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
		if (FAILED(m_Compiler->Compile(&buffer, args, _countof(args), m_IncludeHandler.Get(), IID_PPV_ARGS(result.GetAddressOf()))))
		{
			CORE_ERROR("Failed to compile shader");
			return {};
		}

		wrl::ComPtr<IDxcBlobUtf8> errors;
		if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr)))
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
			CORE_ERROR("Failed to compile shader \"{0}\"", file.string());
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

	void DirectX12ShaderCompiler::GetShaderParameters(ShaderBlobs& blobs, std::vector<Engine::ShaderParameter>& params,  ShaderType type)
	{
		if (!blobs.reflection)
			return;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		wrl::ComPtr<ID3D12ShaderReflection> reflection;
		CORE_ASSERT_HRESULT(m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		// bound resources
		params.reserve(reflectionDesc.BoundResources);
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

			if (bindDesc.BindCount > 1 || bindDesc.Type == D3D_SIT_TEXTURE || bindDesc.Type == D3D_SIT_UAV_RWTYPED)
				data.type = ShaderParameter::PerameterType::DescriptorTable;
			else
			{
				if (bindDesc.Type == D3D_SIT_SAMPLER)
					data.type = ShaderParameter::PerameterType::StaticSampler;
				else
				{
					if (data.name.rfind("RC_", 0) == 0)
						data.type = ShaderParameter::PerameterType::Constants;
					else
						data.type = ShaderParameter::PerameterType::Descriptor;
				}
			}

			switch (bindDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				data.descType = ShaderParameter::DescriptorType::CBV; break;
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_TEXTURE:
				data.descType = ShaderParameter::DescriptorType::SRV; break;
			case D3D_SIT_UAV_RWTYPED:
				data.descType = ShaderParameter::DescriptorType::UAV; break;
			case D3D_SIT_SAMPLER:
				data.descType = ShaderParameter::DescriptorType::Sampler; break;
			default:
				break;
			}

			params.push_back(data);
		}
	}

	void DirectX12ShaderCompiler::GetInputLayout(ShaderBlobs& blobs, std::vector<ShaderInputElement>& inputElements)
	{
		if (!blobs.reflection)
			return;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		wrl::ComPtr<ID3D12ShaderReflection> reflection;
		CORE_ASSERT_HRESULT(m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		inputElements.reserve(reflectionDesc.InputParameters);
		for (uint32 ipIndex = 0; ipIndex < reflectionDesc.InputParameters; ipIndex++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC inputParam;
			reflection->GetInputParameterDesc(ipIndex, &inputParam);

			ShaderInputElement element;
			element.semanticName = inputParam.SemanticName;
			element.semanticIndex = inputParam.SemanticIndex;
			element.format = GetFormatFromDesc(inputParam);
			inputElements.push_back(element);
		}

	}

	void DirectX12ShaderCompiler::GetOutputLayout(ShaderBlobs& blobs, std::vector<FrameBufferTextureFormat>& outputElement)
	{
		if (!blobs.reflection)
			return;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		wrl::ComPtr<ID3D12ShaderReflection> reflection;
		CORE_ASSERT_HRESULT(m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		outputElement.reserve(reflectionDesc.OutputParameters);
		for (uint32 opIndex = 0; opIndex < reflectionDesc.OutputParameters; opIndex++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC outputParam;
			reflection->GetOutputParameterDesc(opIndex, &outputParam);

			switch (outputParam.ComponentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32:
				outputElement.push_back(FrameBufferTextureFormat::RGBA8);
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				outputElement.push_back(FrameBufferTextureFormat::RED_INTEGER);
				break;
			default:
				break;
			}
		}

	}

	wrl::ComPtr<ID3D12RootSignature> DirectX12ShaderCompiler::GenRootSignature(std::vector<ShaderParameter>& params)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsd;
		rsd.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		// create root parameters
		std::vector<D3D12_ROOT_PARAMETER1> rootParams;
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

		uint32 numDescriptorTables = 0;
		for (uint32 i = 0; i < params.size(); i++)
			if (params[i].type == ShaderParameter::PerameterType::DescriptorTable) numDescriptorTables++;

		uint32 currentDescriptor = 0;
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges(numDescriptorTables);

		for (ShaderParameter& rd : params)
		{
			if (rd.type != ShaderParameter::PerameterType::StaticSampler)
			{
				// populate root index
				rd.rootIndex = (uint32)rootParams.size();

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

				rootParams.push_back(param);
			}
			else
			{
				// static samplers
				D3D12_STATIC_SAMPLER_DESC ssd{};
				ssd.ShaderVisibility = GetShaderVisibilityFlag(rd.shader);
				ssd.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				ssd.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				ssd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				if (rd.name.rfind("A_", 0) == 0)
				{
					ssd.Filter = D3D12_FILTER_ANISOTROPIC;
					ssd.MaxLOD = 10;
					ssd.MinLOD = 0;
				}
				else if (rd.name.rfind("P_", 0) == 0)
					ssd.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				else
				{
					ssd.Filter = D3D12_FILTER_ANISOTROPIC;
					ssd.MaxLOD = 10;
					ssd.MinLOD = 0;

				}
				ssd.ShaderRegister = rd.reg;
				ssd.RegisterSpace = rd.space;
				samplers.push_back(ssd);
			}
		}

		D3D12_ROOT_SIGNATURE_DESC1 desc{};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumParameters = (uint32)rootParams.size();
		desc.pParameters = rootParams.data();
		desc.NumStaticSamplers = (uint32)samplers.size();
		desc.pStaticSamplers = samplers.data();

		rsd.Desc_1_1 = desc;

		HRESULT hr = S_OK;
		ID3DBlob* rootSig;
		ID3DBlob* errorBlob;
		hr = D3D12SerializeVersionedRootSignature(&rsd, &rootSig, &errorBlob);
		if (FAILED(hr))
		{
			CORE_ERROR((const char*)errorBlob->GetBufferPointer());
			__debugbreak();
		}

		wrl::ComPtr<ID3D12RootSignature> sig;

		hr = context->GetDevice()->CreateRootSignature(0, rootSig->GetBufferPointer(), rootSig->GetBufferSize(), IID_PPV_ARGS(sig.GetAddressOf()));
		CORE_ASSERT_HRESULT(hr, "Failed to create root signature");

		rootSig->Release();
		if (errorBlob) errorBlob->Release();

		return sig;
	}

}

