#pragma once
#include "Engine/Renderer/Shader.h"
#include "DX.h"
#include <dxc/inc/dxcapi.h>
#include <dxc/inc/d3d12shader.h>

namespace Engine
{
	class ENGINE_API DirectX12Shader : public ShaderPass
	{
	public:
		DirectX12Shader(Ref<ShaderSorce> src, const std::string& passName);
		virtual ~DirectX12Shader();

		virtual std::vector<ShaderParameter> GetReflectionData() const override;

		std::vector<ShaderPass::Uniform> GetUniforms();

		wrl::ComPtr<ID3D12RootSignature> GetRootSignature() { return m_Sig; }
		wrl::ComPtr<ID3D12PipelineState> GetPipelineState() { return m_State; }

		ShaderConfig::Topology GetTopologyType() { return m_Src->config.topology; }

	private:
		virtual uint32 GetUniformLocation(const std::string& name) const;

		struct ShaderBlobs
		{
			IDxcBlob* object{nullptr};
			IDxcBlob* reflection{nullptr};
		};

		void Init();
		ShaderBlobs Compile(ShaderType type, const std::string& code);
		void Reflect(ShaderBlobs blobs, ShaderType type);
		void CreateRootSigniture();

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

		uint32 m_NumRootBindings = 0;

		std::vector<ShaderInputElement> m_InputElements;
		std::vector<ShaderParameter> m_ReflectionData;
		std::vector<FrameBufferTextureFormat> m_RenderTargetFormates;

		wrl::ComPtr<ID3D12RootSignature> m_Sig;
		wrl::ComPtr<ID3D12PipelineState> m_State;

		static wrl::ComPtr<IDxcCompiler3> s_Compiler;
		static wrl::ComPtr<IDxcUtils> s_Utils;
		static wrl::ComPtr<IDxcIncludeHandler> s_IncludeHandler;
	};
}
