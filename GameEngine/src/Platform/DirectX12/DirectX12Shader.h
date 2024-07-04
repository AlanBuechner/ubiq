#pragma once
#include "Engine/Renderer/Abstractions/Shader.h"
#include "DirectX12ShaderCompiler.h"
#include "DX.h"
#include <unordered_map>

namespace Engine
{
	class FrameBuffer;
}

namespace Engine
{
	class ENGINE_API DirectX12Shader : public ShaderPass
	{
	public:
		DirectX12Shader(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12Shader();

		virtual std::vector<ShaderParameter> GetReflectionData() const override;

		wrl::ComPtr<ID3D12RootSignature> GetRootSignature() { return m_Sig; }
		wrl::ComPtr<ID3D12PipelineState> GetPipelineState(Ref<FrameBuffer> target);

		ShaderConfig::RenderPass::Topology GetTopologyType() { return m_PassConfig.topology; }

		virtual uint32 GetUniformLocation(const std::string& name) const override;
	private:

		void Init();

		struct ByteCodeBlobs
		{
			IDxcBlob* vs;
			IDxcBlob* ps;
			IDxcBlob* cs;

			operator bool()
			{
				if (cs != nullptr) return true;
				else
				{
					if (vs && ps) return true;
				}
				return false;
			}
		} m_Blobs;
		wrl::ComPtr<ID3D12PipelineState> CreatePiplineState(const std::vector<TextureFormat>& formats);


	private:
		Ref<ShaderSorce> m_Src;
		std::string m_PassName;
		ShaderConfig::RenderPass& m_PassConfig;

		struct FBVectorHash {
			std::size_t operator()(const std::vector<TextureFormat>& c) const {
				std::hash<uint32> hasher;
				size_t seed = 0;
				for (TextureFormat i : c) {
					seed ^= hasher((uint32)i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
				return seed;
			}
		};


		std::unordered_map<std::vector<TextureFormat>, wrl::ComPtr<ID3D12PipelineState>, FBVectorHash> m_PiplineStates;
		std::unordered_map<std::string, uint32> m_UniformLocations;

		std::vector<ShaderInputElement> m_InputElements;
		std::vector<ShaderParameter> m_ReflectionData;
		std::vector<TextureFormat> m_RenderTargetFormates;

		ID3D12RootSignature* m_Sig;
	};
}
