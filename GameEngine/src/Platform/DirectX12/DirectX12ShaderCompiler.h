#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/ShaderCompiler.h"
#include "DX.h"
#include <dxc/inc/dxcapi.h>
#include <dxc/inc/d3d12shader.h>

namespace Engine
{
	struct ShaderBlobs
	{
		IDxcBlob* object{ nullptr };
		IDxcBlob* reflection{ nullptr };
	};

	class DirectX12ShaderCompiler
	{
		static DirectX12ShaderCompiler* s_Instance;
	public:

		DirectX12ShaderCompiler();
		static DirectX12ShaderCompiler& Get();

		ShaderBlobs Compile(const std::string& code, const fs::path& file, ShaderType type);
		void GetShaderParameters(ShaderBlobs& blobs, std::vector<ShaderParameter>& params, ShaderType type);
		void GetInputLayout(ShaderBlobs& blobs, std::vector<ShaderInputElement>& inputElements);
		void GetOutputLayout(ShaderBlobs& blobs, std::vector<FrameBufferTextureFormat>& outputElement);

		wrl::ComPtr<ID3D12RootSignature> GenRootSignature(std::vector<ShaderParameter>& params);

		wrl::ComPtr<IDxcUtils> GetUtils() { return m_Utils; }

	private:
		wrl::ComPtr<IDxcCompiler3> m_Compiler;
		wrl::ComPtr<IDxcUtils> m_Utils;
		wrl::ComPtr<IDxcIncludeHandler> m_IncludeHandler;
	};
}