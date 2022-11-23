#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shader.h"
#include "DirectX12ShaderCompiler.h"
#include "DX.h"

namespace Engine
{
	class DirectX12ComputeShader : public ComputeShader
	{
	public:
		DirectX12ComputeShader(const fs::path& file);

		wrl::ComPtr<ID3D12RootSignature> GetRootSignature() { return m_Sig; }
		wrl::ComPtr<ID3D12PipelineState> GetPipelineState() { return m_State; }

		std::vector<ShaderParameter> GetReflectionData() const override;
		uint32 GetUniformLocation(const std::string& name) const override;

	private:
		void GenPiplineState(IDxcBlob* blob);

	private:
		fs::path m_File;

		uint32 m_NumRootBindings = 0;

		std::unordered_map<std::string, uint32> m_UniformLocations;
		std::vector<ShaderParameter> m_ReflectionData;

		wrl::ComPtr<ID3D12RootSignature> m_Sig;
		wrl::ComPtr<ID3D12PipelineState> m_State;
	};
}
