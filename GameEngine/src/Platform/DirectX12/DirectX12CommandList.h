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
		virtual void SignalRecording() override { m_RecordFlag.Signal(); }
		virtual void StartRecording() override;

		// transitions
		void Present(FrameBufferState from) { Present(nullptr, from); }
		virtual void Present(Ref<FrameBuffer> fb, FrameBufferState from) override;

		virtual void Transition(std::vector<Ref<FrameBuffer>> fbs, FrameBufferState to, FrameBufferState from) override;
		virtual void Transition(std::vector<FBTransitionObject> transitions) override;

		// rendering
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) override;

		virtual void ClearRenderTarget() override;
		virtual void ClearRenderTarget(uint32 attachment) override;
		virtual void ClearRenderTarget(uint32 attachment, const Math::Vector4& color) override;

		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer) override;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment) override;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color) override;

		virtual void SetShader(Ref<ShaderPass> shader) override;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) override;
		virtual void SetConstantBuffer(uint32 index, Ref<RWConstantBuffer> buffer) override;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) override;
		virtual void SetRootConstant(uint32 index, uint32 data) override;
		virtual void SetTexture(uint32 index, Ref<Texture> texture) override;
		virtual void SetFrameBuffer(uint32 index, Ref<FrameBuffer> buffer, uint32 attatchment) override;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) override;
		virtual void ExecuteBundle(Ref<CommandList> commandList) override;

		virtual void SetComputeShader(Ref<ComputeShader> shader) override;

		virtual void Close() override;

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
