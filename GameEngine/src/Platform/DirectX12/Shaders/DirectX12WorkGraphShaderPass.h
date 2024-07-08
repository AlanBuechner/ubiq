#pragma once
#include "Engine/Renderer/Shaders/WorkGraphShaderPass.h"
#include "Platform/DirectX12/DX.h"
#include "DirectX12ShaderCompiler.h"
#include <unordered_map>


namespace Engine
{
	class DirectX12WorkGraphShaderPass : public WorkGraphShaderPass
	{
	public:
		DirectX12WorkGraphShaderPass(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12WorkGraphShaderPass();

		ID3D12RootSignature* GetRootSignature() { return m_Sig; }
		ID3D12StateObject* GetPipelineState() { return m_Pipline; }
		D3D12_PROGRAM_IDENTIFIER GetIdentifier() { return m_Identifier; }
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE GetBackingMemory() { return m_BackingMemoryAddress; }

	private:

		void Init();

		struct ByteCodeBlobs
		{
			IDxcBlob* wg;

			operator bool()
			{
				if (wg != nullptr) return true;
				return false;
			}
		} m_Blobs;
		ID3D12StateObject* CreatePiplineState();
		void CreateBackingMemory();

	private:
		ID3D12StateObject* m_Pipline;
		ID3D12RootSignature* m_Sig;

		ID3D12Resource* m_BackingMemory;
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE m_BackingMemoryAddress = {};
		D3D12_PROGRAM_IDENTIFIER m_Identifier = {};
		D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS m_MemReqs = {};
	};
}
