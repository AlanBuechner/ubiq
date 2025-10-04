#pragma once
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "DX.h"

#include "tracy/TracyD3D12.hpp"

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

		void FlushResourceBarriers();

		void BeginEvent(const char* eventName);
		void EndEvent();

		void BeginGPUEvent(const tracy::SourceLocationData* data);
		void EndGPUEvent();

		// transitions
		void Transition(const CPUResourceTransitionCommand& cmd);

		// UAVs
		void AwaitUAVs(const CPUAwaitUAVCommand& cmd);

		// transient allocation
		void OpenTransient(const CPUOpenTransientCommand& cmd);
		void CloseTransient(const CPUCloseTransientCommand& cmd);

		// msaa
		void ResolveMSAA(const CPUResolveMSAACommand& cmd);

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
		void BindData(const CPUBindDataCommand& cmd);
		void SetRootConstant(const GPUDataBinding& binding);
		void SetConstantBuffer(const GPUDataBinding& binding);
		void SetStructuredBuffer(const GPUDataBinding& binding);
		void SetRWStructuredBuffer(const GPUDataBinding& binding);
		void SetTexture(const GPUDataBinding& binding);
		void SetRWTexture(const GPUDataBinding& binding);

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

		Utils::Vector<D3D12_RESOURCE_BARRIER> m_ResourceBarriers;

		Utils::Vector<std::string> m_EventStack;
		Utils::Vector<tracy::D3D12ZoneScope*> m_TracyEventStack;

		friend class DirectX12CommandQueue;
	};
}
