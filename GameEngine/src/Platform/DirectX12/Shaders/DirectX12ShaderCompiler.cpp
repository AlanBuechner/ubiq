#include "pch.h"
#include "DirectX12ShaderCompiler.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "dxcapi.h"


Engine::DirectX12ShaderCompiler* Engine::DirectX12ShaderCompiler::s_Instance;


Engine::UniformType GetFormatFromDesc(D3D12_SIGNATURE_PARAMETER_DESC& desc)
{
	if (desc.Mask == 1)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::UniformType::Uint;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::UniformType::Int;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::UniformType::Float;
	}
	else if (desc.Mask <= 3)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::UniformType::Uint2;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::UniformType::Int2;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::UniformType::Float2;
	}
	else if (desc.Mask <= 7)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::UniformType::Uint3;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::UniformType::Int3;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::UniformType::Float3;
	}
	else if (desc.Mask <= 15)
	{
		if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		return Engine::UniformType::Uint4;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	return Engine::UniformType::Int4;
		else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	return Engine::UniformType::Float4;
	}
	CORE_WARN("shader formate unknown defaulting to float");
	return Engine::UniformType::Float;
}

D3D12_SHADER_VISIBILITY GetShaderVisibilityFlag(Engine::ShaderType type)
{
	switch (type)
	{
	case Engine::ShaderType::Vertex:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case Engine::ShaderType::Pixel:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	case Engine::ShaderType::Geometry:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case Engine::ShaderType::Compute:
	default:
		return D3D12_SHADER_VISIBILITY_ALL;
		break;
	}
}

D3D12_TEXTURE_ADDRESS_MODE GetWrapMode(Engine::WrapMode mode)
{
	switch (mode)
	{
	case Engine::WrapMode::Repeat:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case Engine::WrapMode::MirroredRepeat:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case Engine::WrapMode::Clamp:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	default:
		break;
	}
}

D3D12_FILTER GetFilter(Engine::MinMagFilter min, Engine::MinMagFilter mag)
{
	if (min == mag)
	{
		switch (min)
		{
		case Engine::MinMagFilter::Point:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case Engine::MinMagFilter::Linear:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case Engine::MinMagFilter::Anisotropic:
			return D3D12_FILTER_ANISOTROPIC;
		default:
			break;
		}
	}

	CORE_ASSERT(false, "unknown sampler filter config");
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

namespace Engine
{

	DirectX12ShaderCompiler::DirectX12ShaderCompiler()
	{
		static HMODULE s_hmod = 0;
		static HMODULE s_hmodDxil = 0;
		static DxcCreateInstanceProc s_pDxcCreateInstanceProc = nullptr;
		if (s_hmodDxil == 0)
		{
			s_hmodDxil = LoadLibrary(L"dxil.dll");
			CORE_ASSERT(s_hmodDxil != 0, "dxil.dll missing or wrong architecture");
		}
		if (s_hmod == 0)
		{
			s_hmod = LoadLibrary(L"dxcompiler.dll");
			CORE_ASSERT(s_hmod != 0, "dxcompiler.dll missing or wrong architecture");

			if (s_pDxcCreateInstanceProc == nullptr)
			{
				s_pDxcCreateInstanceProc = (DxcCreateInstanceProc)GetProcAddress(s_hmod, "DxcCreateInstance");
				CORE_ASSERT(s_pDxcCreateInstanceProc != nullptr, "Unable to find dxcompiler!DxcCreateInstance");
			}
		}

		CORE_ASSERT_HRESULT(s_pDxcCreateInstanceProc(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.GetAddressOf())), "Failed to create compiler");
		CORE_ASSERT_HRESULT(s_pDxcCreateInstanceProc(CLSID_DxcUtils, IID_PPV_ARGS(m_Utils.GetAddressOf())), "Failed To Create compiler utils");
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
		CREATE_PROFILE_SCOPEI("Compile Shader");
		const std::wstring typeStrings[] = { L"vs_6_5", L"ps_6_5", L"gs_6_5", L"cs_6_5", L"lib_6_8" };
		const std::wstring& profile = typeStrings[(int)type]; // take the log2 of the type enum to convert from bit mask to index
		//std::wstring profile = typeStrings[(int)log2((int)type)]; // take the log2 of the type enum to convert from bit mask to index

		Utils::Vector<LPCWSTR> args;
		args.Push(file.c_str());
		if (type != ShaderType::WorkGraph)
		{
			args.Push(L"-E");
			args.Push(L"main");
		}
		args.Push(L"-T");
		args.Push(profile.c_str());
		args.Push(DXC_ARG_ALL_RESOURCES_BOUND);
#ifdef DEBUG
		args.Push(DXC_ARG_SKIP_OPTIMIZATIONS);
#else
		args.Push(DXC_ARG_SKIP_OPTIMIZATIONS);
		//args.Push(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif // DEBUG
		args.Push(DXC_ARG_DEBUG);
		args.Push(L"-Qembed_debug"); // embed debug information

		DxcBuffer buffer;
		buffer.Encoding = DXC_CP_ACP;
		buffer.Ptr = code.c_str();
		buffer.Size = code.size();

		wrl::ComPtr<IDxcResult> result;
		if (FAILED(m_Compiler->Compile(&buffer, args.Data(), args.Count(), m_IncludeHandler.Get(), IID_PPV_ARGS(result.GetAddressOf()))))
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

	wrl::ComPtr<ID3D12ShaderReflection> DirectX12ShaderCompiler::GetShaderReflection(const ShaderBlobs& blobs)
	{
		if (!blobs.reflection)
			return nullptr;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		wrl::ComPtr<ID3D12ShaderReflection> reflection;
		CORE_ASSERT_HRESULT(m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		return reflection;
	}

	wrl::ComPtr<ID3D12LibraryReflection> DirectX12ShaderCompiler::GetLibraryReflection(const ShaderBlobs& blobs)
	{
		if (!blobs.reflection)
			return nullptr;

		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = blobs.reflection->GetBufferPointer();
		reflectionData.Size = blobs.reflection->GetBufferSize();

		ID3D12LibraryReflection* reflection;
		CORE_ASSERT_HRESULT(m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)), "Failed to create reflection data");

		return reflection;
	}

	void DirectX12ShaderCompiler::GetComputeGroupSize(wrl::ComPtr<ID3D12ShaderReflection> reflection, Math::UVector3& groupSize)
	{
		reflection->GetThreadGroupSize(&groupSize.x, &groupSize.y, &groupSize.z);
		groupSize.x = Math::Max(groupSize.x, 1u);
		groupSize.y = Math::Max(groupSize.y, 1u);
		groupSize.z = Math::Max(groupSize.z, 1u);
	}

	void DirectX12ShaderCompiler::GetShaderParameters(wrl::ComPtr<ID3D12ShaderReflection> reflection, const SectionInfo& section, Utils::Vector<Engine::ShaderParameter>& params, ShaderType type)
	{
		CREATE_PROFILE_SCOPEI("Get Shader Parameters");
		
		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		// bound resources
		params.Reserve(reflectionDesc.BoundResources);
		for (uint32 srvIndex = 0; srvIndex < reflectionDesc.BoundResources; srvIndex++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(srvIndex, &bindDesc);
			AddParameter(bindDesc, section, params, type);
		}
	}

	void DirectX12ShaderCompiler::GetLibraryParameters(wrl::ComPtr<ID3D12LibraryReflection> reflection, const SectionInfo& section, Utils::Vector<ShaderParameter>& params, ShaderType type)
	{
		CREATE_PROFILE_SCOPEI("Get Library Parameters");
		
		D3D12_LIBRARY_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		for (uint32 funcIndex = 0; funcIndex < reflectionDesc.FunctionCount; funcIndex++)
		{
			ID3D12FunctionReflection* funcReflection = reflection->GetFunctionByIndex(funcIndex);

			D3D12_FUNCTION_DESC funcDesc;
			CORE_ASSERT_HRESULT(funcReflection->GetDesc(&funcDesc), "Failed to get function descriptor");

			for (uint32 srvIndex = 0; srvIndex < funcDesc.BoundResources; srvIndex++)
			{
				D3D12_SHADER_INPUT_BIND_DESC bindDesc;
				funcReflection->GetResourceBindingDesc(srvIndex, &bindDesc);
				AddParameter(bindDesc, section, params, type);
			}

		}
	}

	void DirectX12ShaderCompiler::GetInputLayout(wrl::ComPtr<ID3D12ShaderReflection> reflection, Utils::Vector<ShaderInputElement>& inputElements)
	{
		CREATE_PROFILE_SCOPEI("Get Input Layout");

		D3D12_SHADER_DESC reflectionDesc;
		reflection->GetDesc(&reflectionDesc);

		inputElements.Reserve(reflectionDesc.InputParameters);
		for (uint32 ipIndex = 0; ipIndex < reflectionDesc.InputParameters; ipIndex++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC inputParam;
			reflection->GetInputParameterDesc(ipIndex, &inputParam);

			ShaderInputElement element;
			element.semanticName = inputParam.SemanticName;
			element.semanticIndex = inputParam.SemanticIndex;
			element.format = GetFormatFromDesc(inputParam);
			inputElements.Push(element);
		}

	}

	ID3D12RootSignature* DirectX12ShaderCompiler::GenRootSignature(Utils::Vector<ShaderParameter>& params)
	{
		CREATE_PROFILE_SCOPEI("Generate Root Signature");
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsd;
		rsd.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

		// create root parameters
		Utils::Vector<D3D12_ROOT_PARAMETER1> rootParams;
		Utils::Vector<D3D12_STATIC_SAMPLER_DESC> samplers;

		// find number of descriptor tables
		uint32 numDescriptorTables = 0;
		for (uint32 i = 0; i < params.Count(); i++)
			if (params[i].type == PerameterType::DescriptorTable) numDescriptorTables++;

		// track descriptor ranges for descriptor tables
		uint32 currentDescriptor = 0;
		Utils::Vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges(numDescriptorTables);

		// find all shared shader parameters
		std::unordered_set<std::string> sharedParamNames;
		Utils::Vector<uint32> paramsToSkip;
		for (uint32 i = 0; i < params.Count() - 1; i++)
		{
			// get current param
			ShaderParameter& param = params[i];

			// find shared
			bool isShared = false;
			for (uint32 j = i + 1; j < params.Count(); j++)
			{
				// skip automatically bound parameters
				if(param.type == PerameterType::DescriptorTable || param.type == PerameterType::StaticSampler)
					continue;

				ShaderParameter& sharedParam = params[j];
				if (param.name == sharedParam.name)
				{
					CORE_ASSERT(param.reg == sharedParam.reg, "Shared shader parameters need the same binding");
					CORE_ASSERT(param.space == sharedParam.space, "Shared shader parameters need the same binding");

					paramsToSkip.Push(j);
					isShared = true;
				}
			}

			if (isShared)
				sharedParamNames.insert(param.name);
		}

		// get root parameters
		for (uint32 i = 0; i < params.Count(); i++)
		{
			// get current param
			ShaderParameter& rd = params[i];

			// check if param needs to be skipped
			if(paramsToSkip.Contains(i))
				continue;

			if (rd.type != PerameterType::StaticSampler)
			{
				// populate root index
				rd.rootIndex = (uint32)rootParams.Count();

				// root parameters
				D3D12_ROOT_PARAMETER1 param;

				// check if param is shared
				if (sharedParamNames.find(rd.name) != sharedParamNames.end())
					param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				else
					param.ShaderVisibility = GetShaderVisibilityFlag(rd.shader);

				if (rd.type == PerameterType::Constants)
				{
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
					param.Constants.Num32BitValues = rd.count;
					param.Constants.ShaderRegister = rd.reg;
					param.Constants.RegisterSpace = rd.space;
				}
				else if (rd.type == PerameterType::Descriptor)
				{
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					switch (rd.descType)
					{
					case DescriptorType::CBV:	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; break;
					case DescriptorType::SRV:	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; break;
					case DescriptorType::UAV:	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV; break;
					default:
						break;
					}
					param.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
					param.Descriptor.ShaderRegister = rd.reg;
					param.Descriptor.RegisterSpace = rd.space;
				}
				else if (rd.type == PerameterType::DescriptorTable)
				{
					D3D12_DESCRIPTOR_RANGE_TYPE type;
					switch (rd.descType)
					{
					case DescriptorType::CBV:		type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					case DescriptorType::SRV:		type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case DescriptorType::UAV:		type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case DescriptorType::Sampler:	type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
					default:
						break;
					}

					descriptorRanges[currentDescriptor].Init(type, rd.count, rd.reg, rd.space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					param.DescriptorTable.NumDescriptorRanges = 1;
					param.DescriptorTable.pDescriptorRanges = &descriptorRanges[currentDescriptor];
					currentDescriptor++;
				}

				rootParams.Push(param);
			}
			else
			{
				// static samplers
				D3D12_STATIC_SAMPLER_DESC ssd{};
				ssd.ShaderVisibility = GetShaderVisibilityFlag(rd.shader);

				SamplerInfo& attrib = rd.samplerAttribs;
				ssd.AddressU = GetWrapMode(attrib.U);
				ssd.AddressV = GetWrapMode(attrib.V);
				ssd.AddressW = GetWrapMode(WrapMode::Clamp);

				ssd.Filter = GetFilter(attrib.Min, attrib.Mag);
				ssd.MaxAnisotropy = 16;

				ssd.MinLOD = 0;
				ssd.MaxLOD = 10;

				ssd.ShaderRegister = rd.reg;
				ssd.RegisterSpace = rd.space;
				samplers.Push(ssd);
			}
		}

		D3D12_ROOT_SIGNATURE_DESC1 desc{};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumParameters = (uint32)rootParams.Count();
		desc.pParameters = rootParams.Data();
		desc.NumStaticSamplers = (uint32)samplers.Count();
		desc.pStaticSamplers = samplers.Data();

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

		ID3D12RootSignature* sig;

		hr = context->GetDevice()->CreateRootSignature(0, rootSig->GetBufferPointer(), rootSig->GetBufferSize(), IID_PPV_ARGS(&sig));
		CORE_ASSERT_HRESULT(hr, "Failed to create root signature");

		rootSig->Release();
		if (errorBlob) errorBlob->Release();

		return sig;
	}

	void DirectX12ShaderCompiler::AddParameter(D3D12_SHADER_INPUT_BIND_DESC bindDesc, const SectionInfo& section, Utils::Vector<ShaderParameter>& params, ShaderType type)
	{
		CREATE_PROFILE_SCOPEI("Add Parameter");
		ANOTATE_PROFILEI("Parameter Name: " + std::string(bindDesc.Name));
		ShaderParameter data;
		data.shader = type;
		data.name = bindDesc.Name;
		data.reg = bindDesc.BindPoint;
		data.space = bindDesc.Space;
		data.count = 0; // default value

		if (bindDesc.BindCount > 1 ||
			bindDesc.Type == D3D_SIT_TEXTURE ||
			bindDesc.Type == D3D_SIT_UAV_RWTYPED ||
			bindDesc.Type == D3D_SIT_STRUCTURED ||
			bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
		{
			data.type = PerameterType::DescriptorTable;
			data.count = bindDesc.BindCount;
			if (data.count == 0)
				data.count = -1;
		}
		else
		{
			if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				data.type = PerameterType::StaticSampler;

				if (section.m_Samplers.find(data.name) != section.m_Samplers.end())
					data.samplerAttribs = section.m_Samplers.at(data.name);
				else
				{
					// TODO : non static sampler
				}
			}
			else
			{

				if (section.m_RootConstants.find(data.name) != section.m_RootConstants.end())
				{
					data.type = PerameterType::Constants;
					data.count = 1;
				}
				else
					data.type = PerameterType::Descriptor;
			}
		}

		switch (bindDesc.Type)
		{
		case D3D_SIT_CBUFFER:		data.descType = DescriptorType::CBV; break;

		case D3D_SIT_STRUCTURED:
		case D3D_SIT_TEXTURE:		data.descType = DescriptorType::SRV; break;

		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWTYPED:	data.descType = DescriptorType::UAV; break;
		case D3D_SIT_SAMPLER:		data.descType = DescriptorType::Sampler; break;
		default:
			break;
		}

		for (uint32 i = 0; i < params.Count(); i++)
		{
			if (params[i] == data)
				return;
		}

		params.Push(data);
	}

}

