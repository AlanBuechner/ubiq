#pragma once
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "DX.h"

namespace Engine
{
	class DirectX12CommandList : public CommandList
	{
	public:
		DirectX12CommandList(CommandListType type) :
			CommandList(type)
		{ Init(); }

		

	private:
		void Init();

	protected:
		virtual void InternalClose() override;

	public:
		virtual void StartRecording() override;

		// transitions
		void Present() { Present(nullptr); }
		virtual void Present(Ref<FrameBuffer> fb) override;

		virtual void Transition(std::vector<ResourceTransitionObject> transitions) override;

		virtual void ValidateStates(std::vector<ResourceStateObject> resources) override;


		// copying
		virtual void CopyBuffer(GPUResource* dest, uint64 destOffset, GPUResource* src, uint64 srcOffset, uint64 size) override;
		virtual void CopyResource(GPUResource* dest, GPUResource* src) override;

		virtual void UploadTexture(GPUResource* dest, UploadTextureResource* src) override;

		// rendering
		virtual void SetRenderTarget(Ref<RenderTarget2D> renderTarget) override;
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) override;

		virtual void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color) override;

		void InitalizeDescriptorTables(Ref<ShaderPass> shader);
		virtual void SetShader(Ref<GraphicsShaderPass> shader) override;
		virtual void SetShader(Ref<ComputeShaderPass> shader) override;
		virtual void SetShader(Ref<WorkGraphShaderPass> shader) override;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) override;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) override;
		virtual void SetRootConstant(uint32 index, uint32 data) override;
		virtual void SetTexture(uint32 index, Ref<Texture2D> texture) override;
		virtual void SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav) override;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) override;
		virtual void ExecuteBundle(Ref<CommandList> commandList) override;

		virtual void Dispatch(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGrouptsZ) override;

		virtual void AwaitUAV(GPUResource* uav) override;

		virtual void Close() override;

		std::vector<ResourceStateObject>& GetPendingTransitions() override { return m_Frames[GetLastFrameIndex()].pendingTransitions; }
		std::unordered_map<GPUResource*, ResourceState> GetEndingResourceStates() override { return m_Frames[GetLastFrameIndex()].resourceStates; }
		ID3D12GraphicsCommandList10* GetCommandList() { return m_CommandList; }

	private:
		std::vector<ResourceStateObject>& GetCurrentPendingTransitions() { return m_Frames[m_CurrentFrame].pendingTransitions; }
		ID3D12CommandList* GetPrependCommandList() { return m_PrependList; }

		bool RecordPrependCommands();

	private:
		ID3D12CommandAllocator* GetAllocator() { return m_Frames[m_CurrentFrame].commandAllocator; }
		std::unordered_map<GPUResource*, ResourceState>& GetResourceStates() { return m_Frames[m_CurrentFrame].resourceStates; }

		uint32 GetLastFrameIndex() { return (m_CurrentFrame - 1) % m_Frames.size(); }

	private:

		struct RecordFrame
		{
			ID3D12CommandAllocator* commandAllocator;
			std::vector<ResourceStateObject> pendingTransitions;
			std::unordered_map<GPUResource*, ResourceState> resourceStates;
		};

		uint32 m_CurrentFrame = 0;
		ID3D12GraphicsCommandList10* m_CommandList;
		std::vector<RecordFrame> m_Frames;

		ID3D12CommandAllocator* m_PrependAllocator;
		ID3D12GraphicsCommandList10* m_PrependList;

		friend class DirectX12CommandQueue;
	};
}
