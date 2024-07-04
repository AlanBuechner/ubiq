#pragma once
#include "Engine/Renderer/Shaders/ComputeShaderPass.h"
#include "Platform/DirectX12/DX.h"
#include "DirectX12ShaderCompiler.h"
#include <unordered_map>


namespace Engine
{
	class DirectX12ComputeShaderPass : public ComputeShaderPass
	{
	public:
		DirectX12ComputeShaderPass(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12ComputeShaderPass();

		ID3D12RootSignature* GetRootSignature() { return m_Sig; }
		ID3D12PipelineState* GetPipelineState() { return m_Pipline; }

	private:

		void Init();

		struct ByteCodeBlobs
		{
			IDxcBlob* cs;

			operator bool()
			{
				if (cs != nullptr) return true;
				return false;
			}
		} m_Blobs;
		ID3D12PipelineState* CreatePiplineState();

	private:
		ID3D12PipelineState* m_Pipline;
		ID3D12RootSignature* m_Sig;
	};
}
