#pragma once
#include "Engine/Renderer/Shaders/GraphicsShaderPass.h"
#include "Platform/DirectX12/DX.h"
#include "DirectX12ShaderCompiler.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include <unordered_map>

namespace Engine
{
	class DirectX12GraphicsShaderPass : public GraphicsShaderPass
	{
	public:
		DirectX12GraphicsShaderPass(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12GraphicsShaderPass();

		ID3D12RootSignature* GetRootSignature() { return m_Sig; }
		ID3D12PipelineState* GetPipelineState(const FrameBufferDescription& fbDesc);

		Topology GetTopologyType() { return m_PassConfig.topology; }
	private:

		void Init();

		struct ByteCodeBlobs
		{
			IDxcBlob* vs;
			IDxcBlob* ps;
			IDxcBlob* gs = nullptr;

			operator bool()
			{
				if (vs && ps) return true;
				return false;
			}
		} m_Blobs;
		ID3D12PipelineState* CreatePiplineState(const FrameBufferDescription& fbDesc);


	private:
		std::unordered_map<FrameBufferDescription, ID3D12PipelineState*, FBDHash> m_PiplineStates;

		Utils::Vector<ShaderInputElement> m_InputElements;

		ID3D12RootSignature* m_Sig;
	};
}
