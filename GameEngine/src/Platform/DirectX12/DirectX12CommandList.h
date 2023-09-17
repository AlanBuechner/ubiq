#pragma once
#include "Engine/Renderer/CommandList.h"
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

	protected:
		virtual void SignalRecording() override { m_RecordFlag.Signal(); }
		virtual void InternalClose() override;

	public:
		virtual void StartRecording() override;

		// transitions
		void Present() { Present(nullptr); }
		virtual void Present(Ref<FrameBuffer> fb) override;

		virtual void Transition(std::vector<ResourceTransitionObject> transitions) override;

		virtual void ValidateStates(std::vector<ResourceStateObject> resources) override;

		// rendering
		virtual void SetRenderTarget(Ref<RenderTarget2D> renderTarget) override;
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) override;

		virtual void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color) override;

		virtual void SetShader(Ref<ShaderPass> shader) override;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) override;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) override;
		virtual void SetRootConstant(uint32 index, uint32 data) override;
		virtual void SetTexture(uint32 index, Ref<Texture2D> texture) override;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) override;
		virtual void ExecuteBundle(Ref<CommandList> commandList) override;

		virtual void Dispatch(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGrouptsZ) override;

		virtual void Close() override;

		std::vector<ResourceStateObject>& GetPendingTransitions() { return m_Frames[GetLastFrameIndex()].pendingTransitions; }
		std::unordered_map<GPUResource*, ResourceState> GetEndingResourceStates() { return m_Frames[GetLastFrameIndex()].resourceStates; }
		wrl::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_CommandList; }

	private:
		std::vector<ResourceStateObject>& GetCurrentPendingTransitions() { return m_Frames[m_CurrentFrame].pendingTransitions; }
		wrl::ComPtr<ID3D12CommandList> GetPrependCommandList() { return m_PrependList; }

		bool RecordPrependCommands();

	private:
		wrl::ComPtr<ID3D12CommandAllocator>& GetAllocator() { return m_Frames[m_CurrentFrame].commandAllocator; }
		std::unordered_map<GPUResource*, ResourceState>& GetResourceStates() { return m_Frames[m_CurrentFrame].resourceStates; }

		uint32 GetLastFrameIndex() { return (m_CurrentFrame - 1) % m_Frames.size(); }

	private:

		struct RecordFrame
		{
			wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
			std::vector<ResourceStateObject> pendingTransitions;
			std::unordered_map<GPUResource*, ResourceState> resourceStates;
		};

		Ref<FrameBuffer> m_RenderTarget;
		Ref<ShaderPass> m_BoundShader;

		uint32 m_CurrentFrame = 0;
		wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		std::vector<RecordFrame> m_Frames;

		wrl::ComPtr<ID3D12CommandAllocator> m_PrependAllocator;
		wrl::ComPtr<ID3D12GraphicsCommandList> m_PrependList;

		CommandListType m_Type;

		Flag m_RecordFlag;

		RecordState m_State = CommandList::Closed;

		friend class DirectX12CommandQueue;
	};
}
