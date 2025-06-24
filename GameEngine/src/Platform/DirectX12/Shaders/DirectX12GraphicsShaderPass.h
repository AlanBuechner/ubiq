#pragma once
#include "Engine/Renderer/Shaders/GraphicsShaderPass.h"
#include "Platform/DirectX12/DX.h"
#include "DirectX12ShaderCompiler.h"
#include <unordered_map>

namespace Engine
{
	class FrameBuffer;
}

struct FBVectorHash {
	std::size_t operator()(const Utils::Vector<Engine::TextureFormat>& c) const {
		std::hash<uint32> hasher;
		size_t seed = 0;
		for (Engine::TextureFormat i : c) {
			seed ^= hasher((uint32)i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

namespace Engine
{
	class DirectX12GraphicsShaderPass : public GraphicsShaderPass
	{
	public:
		DirectX12GraphicsShaderPass(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12GraphicsShaderPass();

		ID3D12RootSignature* GetRootSignature() { return m_Sig; }
		ID3D12PipelineState* GetPipelineState(const Utils::Vector<TextureFormat>& format);

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
		ID3D12PipelineState* CreatePiplineState(const Utils::Vector<TextureFormat>& formats);


	private:
		std::unordered_map<Utils::Vector<TextureFormat>, ID3D12PipelineState*, FBVectorHash> m_PiplineStates;

		Utils::Vector<ShaderInputElement> m_InputElements;

		ID3D12RootSignature* m_Sig;
	};
}

