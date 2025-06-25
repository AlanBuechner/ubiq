#pragma once
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "DX.h"

namespace Engine
{
	class DirectX12CommandList : public CommandList
	{
	public:
		DirectX12CommandList(CommandListType commandListType) :
			CommandList(commandListType)
		{ Init(); }

	private:
		void Init();

	public:
		virtual void StartRecording() override;
		virtual void RecoredCommands(CPUCommandAllocator* commandAllocator) override;
		void Close() override;

		// non command object abstractions
		// NOTE : i am only adding abstractions for imgui
		void Transition(const Utils::Vector<ResourceTransitionObject>& transitions);
		void SetRenderTarget(Ref<RenderTarget2D> renderTarget);
		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, Math::Vector4 color);

		void BeginEvent(const char* eventName);
		void EndEvent();

		// transitions
		void Transition(const CPUResourceTransitionCommand& cmd);

		// UAVs
		void AwaitUAVs(const CPUAwaitUAVCommand& cmd);

		// copying
		void CopyBuffer(const CPUCopyBufferCommand& cmd);
		void UploadTexture(const CPUUploadTextureCommand& Command);

		// set render targets and viewport
		void SetViewport(const CPUSetViewportCommand& cmd);
		void SetRenderTarget(const CPUSetRenderTargetCommand& cmd);

		// clear render targets
		void ClearRenderTarget(const CPUClearRenderTargetCommand& cmd);

		// set shaders
		void InitalizeDescriptorTables(const Ref<ShaderPass> shader);
		void SetShader(const CPUSetGraphicsShaderCommand& cmd);
		void SetShader(const CPUSetComputeShaderCommand& cmd);
		void SetShader(const CPUSetWorkGraphShaderCommand& cmd);

		// set data
		void SetRootConstant(const CPUSetRootConstantCommand& cmd);
		void SetConstantBuffer(const CPUSetConstantBufferCommand& cmd);
		void SetStructuredBuffer(const CPUSetStructuredBufferCommand& cmd);
		void SetRWStructuredBuffer(const CPUSetRWStructuredBufferCommand& cmd);
		void SetTexture(const CPUSetTextureCommand& cmd);
		void SetRWTexture(const CPUSetRWTextureCommand& cmd);

		// do work
		void DrawMesh(const CPUDrawMeshCommand& cmd);
		void Dispatch(const CPUDispatchCommand& cmd);
		void DispatchGraph(const CPUDispatchGraphCPUDataCommand& cmd);
		void DispatchGraph(const CPUDispatchGraphGPUDataCommand& cmd);

		ID3D12CommandAllocator* GetAllocator() { return m_CommandAllocator; }
		ID3D12GraphicsCommandList10* GetCommandList() { return m_CommandList; }

	private:

		ID3D12CommandAllocator* m_CommandAllocator;
		ID3D12GraphicsCommandList10* m_CommandList;

		friend class DirectX12CommandQueue;
	};
}
