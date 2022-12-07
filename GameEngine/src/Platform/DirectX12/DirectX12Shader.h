#pragma once
#include "Engine/Renderer/Shader.h"
#include "DirectX12ShaderCompiler.h"
#include "DX.h"
#include <unordered_map>

namespace Engine
{
	class ENGINE_API DirectX12Shader : public ShaderPass
	{
	public:
		DirectX12Shader(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12Shader();

		virtual std::vector<ShaderParameter> GetReflectionData() const override;

		wrl::ComPtr<ID3D12RootSignature> GetRootSignature() { return m_Sig; }
		wrl::ComPtr<ID3D12PipelineState> GetPipelineState() { return m_State; }

		ShaderConfig::Topology GetTopologyType() { return m_Src->config.topology; }

		virtual uint32 GetUniformLocation(const std::string& name) const override;
	private:

		void Init();

		struct ByteCodeBlobs
		{
			IDxcBlob* vs;
			IDxcBlob* ps;

			operator bool()
			{
				if (vs == nullptr) return false;
				if (ps == nullptr) return false;
				return true;
			}
		};
		void CreatePiplineState(ByteCodeBlobs blobs);


	private:
		Ref<ShaderSorce> m_Src;
		std::string m_PassName;
		ShaderConfig::RenderPass& m_PassConfig;

		std::unordered_map<std::string, uint32> m_UniformLocations;

		std::vector<ShaderInputElement> m_InputElements;
		std::vector<ShaderParameter> m_ReflectionData;
		std::vector<FrameBufferTextureFormat> m_RenderTargetFormates;

		wrl::ComPtr<ID3D12RootSignature> m_Sig;
		wrl::ComPtr<ID3D12PipelineState> m_State;
	};
}
