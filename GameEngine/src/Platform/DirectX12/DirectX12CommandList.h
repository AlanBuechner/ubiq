#pragma once
#include "Engine/Renderer/CommandList.h"
#include "DirectX12FrameBuffer.h"
#include "DX.h"
#include "Engine/Core/Flag.h"

namespace Engine
{
	class DirectX12CommandList : public CommandList
	{
	public:
		DirectX12CommandList(CommandListType type) :
			m_Type(type)
		{ Init(); }

	private:
		void Init();

	public:
		virtual void StartRecording(Ref<ShaderPass> startShader = nullptr) override;

		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) override;

		virtual void ClearRenderTarget() override;
		virtual void ClearRenderTarget(uint32 attachment) override;
		virtual void ClearRenderTarget(uint32 attachment, const Math::Vector4& color) override;

		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer);
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment) override;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color) override;

		virtual void SetShader(Ref<ShaderPass> shader) override;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) override;
		virtual void SetTexture(uint32 index, Ref<Texture> texture) override;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer, int numInstances) override;
		virtual void Present(Ref<FrameBuffer> fb = nullptr) override;
		virtual void ExecuteBundle(Ref<CommandList> commandList) override;
		virtual void Close() override;
		virtual void SignalRecording() override { m_RecordFlag.Signal(); }

		wrl::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_CommandList; }


	private:
		wrl::ComPtr<ID3D12CommandAllocator> GetAllocator();

	private:
		Ref<DirectX12FrameBuffer> m_RenderTarget;

		uint32 m_CurrentCommandAllocator = 0;
		wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		std::vector<wrl::ComPtr<ID3D12CommandAllocator>> m_Allocators;

		CommandListType m_Type;

		Flag m_RecordFlag;
	};
}
