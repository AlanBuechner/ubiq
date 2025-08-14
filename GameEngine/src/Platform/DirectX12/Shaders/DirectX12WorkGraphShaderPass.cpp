#include "DirectX12WorkGraphShaderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Shaders/ShaderCompiler.h"

namespace Engine
{

	DirectX12WorkGraphShaderPass::DirectX12WorkGraphShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		WorkGraphShaderPass(src, passName)
	{
		Init();
	}

	DirectX12WorkGraphShaderPass::~DirectX12WorkGraphShaderPass()
	{
		m_Sig->Release();
		m_Pipline->Release();
		m_BackingMemory->Release();
	}

	void DirectX12WorkGraphShaderPass::Init()
	{
		SectionInfo wgi = m_Src->m_Sections[m_PassConfig.wg];
		const std::string& wgc = wgi.m_SectionCode.str();
		if (!wgc.empty())
		{
			ShaderBlobs wg = DirectX12ShaderCompiler::Get().Compile(wgc, m_Src->file, ShaderType::WorkGraph);
			if (!wg.object) return;
			m_Blobs.wg = wg.object;
			wrl::ComPtr<ID3D12LibraryReflection> reflection = DirectX12ShaderCompiler::Get().GetLibraryReflection(wg);
			CORE_ASSERT(reflection != nullptr, "Failed to get reflection data on shader {0}: {1}", m_Src->file, m_PassName);
			DirectX12ShaderCompiler::Get().GetLibraryParameters(reflection, wgi, m_ReflectionData, ShaderType::WorkGraph);
		}

		if (m_Blobs)
		{
			// create root signature
			m_Sig = DirectX12ShaderCompiler::Get().GenRootSignature(m_ReflectionData);
			CORE_ASSERT(m_Sig, "Failed To Create Root Signature");
			m_Pipline = CreatePiplineState();
			CORE_ASSERT(m_Pipline, "Failed to Create Pipline State Object");
			CreateBackingMemory();

			for (auto& param : m_ReflectionData)
				m_UniformLocations[param.name] = param.rootIndex;
		}
	}

	ID3D12StateObject* DirectX12WorkGraphShaderPass::CreatePiplineState()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		ID3D12StateObject* stateObject;

		CD3DX12_STATE_OBJECT_DESC Desc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

		CD3DX12_STATE_OBJECT_DESC SO(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);
		auto pLib = SO.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		CD3DX12_SHADER_BYTECODE libCode(m_Blobs.wg->GetBufferPointer(), m_Blobs.wg->GetBufferSize());
		pLib->SetDXILLibrary(&libCode);

		auto pRs = SO.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>(); 
		pRs->SetRootSignature(m_Sig);

		auto pWG = SO.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
		pWG->IncludeAllAvailableNodes(); // Auto populate the graph
		LPCWSTR workGraphName = L"HelloWorkGraphs";
		pWG->SetProgramName(workGraphName);

		CORE_ASSERT_HRESULT(context->GetDevice()->CreateStateObject(SO, IID_PPV_ARGS(&stateObject)), "Failed to create state object");

		return stateObject;
	}

	void DirectX12WorkGraphShaderPass::CreateBackingMemory()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		LPCWSTR workGraphName = L"HelloWorkGraphs";

		ID3D12StateObjectProperties1* spSOProps;
		m_Pipline->QueryInterface(IID_PPV_ARGS(&spSOProps));
		m_Identifier = spSOProps->GetProgramIdentifier(workGraphName);
		ID3D12WorkGraphProperties* spWGProps;
		m_Pipline->QueryInterface(IID_PPV_ARGS(&spWGProps));
		UINT WorkGraphIndex = spWGProps->GetWorkGraphIndex(workGraphName);
		spWGProps->GetWorkGraphMemoryRequirements(WorkGraphIndex, &m_MemReqs);
		m_BackingMemoryAddress.SizeInBytes = m_MemReqs.MaxSizeInBytes;

		CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(m_MemReqs.MaxSizeInBytes);
		rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT hr = context->GetDevice()->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&rd,
			D3D12_RESOURCE_STATE_COMMON,
			NULL,
			IID_PPV_ARGS(&m_BackingMemory)
		);
		CORE_ASSERT_HRESULT(hr, "Failed to create backing memory");

		m_BackingMemoryAddress.StartAddress = m_BackingMemory->GetGPUVirtualAddress();
	}

}

