#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shaders/ShaderCompiler.h"
#include "Engine/Renderer/Abstractions/Resources/TextureFormat.h"
#include "Platform/DirectX12/DX.h"
#include <dxcapi.h>
#include <d3d12shader.h>

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
		wrl::ComPtr<ID3D12ShaderReflection> GetShaderReflection(const ShaderBlobs& blobs);
		wrl::ComPtr<ID3D12LibraryReflection> GetLibraryReflection(const ShaderBlobs& blobs);
		void GetComputeGroupSize(wrl::ComPtr<ID3D12ShaderReflection> reflection, Math::UVector3& groupSize);
		void GetShaderParameters(wrl::ComPtr<ID3D12ShaderReflection> reflection, const SectionInfo& section, Utils::Vector<ShaderParameter>& params, ShaderType type);
		void GetLibraryParameters(wrl::ComPtr<ID3D12LibraryReflection> reflection, const SectionInfo& section, Utils::Vector<ShaderParameter>& params, ShaderType type);
		void GetInputLayout(wrl::ComPtr<ID3D12ShaderReflection> reflection, Utils::Vector<ShaderInputElement>& inputElements);

		ID3D12RootSignature* GenRootSignature(Utils::Vector<ShaderParameter>& params);

		wrl::ComPtr<IDxcUtils> GetUtils() { return m_Utils; }

	private:
		void AddParameter(D3D12_SHADER_INPUT_BIND_DESC bindDesc, const SectionInfo& section, Utils::Vector<ShaderParameter>& params, ShaderType type);

	private:
		wrl::ComPtr<IDxcCompiler3> m_Compiler;
		wrl::ComPtr<IDxcUtils> m_Utils;
		wrl::ComPtr<IDxcIncludeHandler> m_IncludeHandler;
	};
}
