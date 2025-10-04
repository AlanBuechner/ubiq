#include "pch.h"
#include "Engine/Math/Math.h"
#include "Engine/Renderer/Renderer.h"

#include "DirectX12CommandList.h"
#include "DirectX12Context.h"
#include "Shaders/DirectX12GraphicsShaderPass.h"
#include "Shaders/DirectX12ComputeShaderPass.h"
#include "Shaders/DirectX12WorkGraphShaderPass.h"
#include "Resources/DirectX12Buffer.h"
#include "Resources/DirectX12ConstantBuffer.h"
#include "Resources/DirectX12StructuredBuffer.h"
#include "Resources/DirectX12Texture.h"
#include "Resources/DirectX12ResourceManager.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "Utils/Performance.h"
#include "Engine/Renderer/Mesh.h"

#include "Utils/Common.h"

namespace Engine
{
	static inline D3D12_RESOURCE_BARRIER TransitionResource(
		_In_ ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
	{
		D3D12_RESOURCE_BARRIER result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = flags;
		result.Transition.pResource = pResource;
		result.Transition.StateBefore = stateBefore;
		result.Transition.StateAfter = stateAfter;
		result.Transition.Subresource = subresource;
		return result;
	}

	static inline D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandListType type)
	{
		switch (type)
		{
		case Engine::CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case Engine::CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case Engine::CommandListType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
		}
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}

	void DirectX12CommandList::Init()
	{

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		const uint32 numAllocators = 2;
		D3D12_COMMAND_LIST_TYPE type = GetD3D12CommandListType(m_CommandListType);

		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_CommandAllocator)),
				"Failed to Create Command List Allocator");
		m_CommandAllocator->SetName(L"Command Allocator");
		
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandList(0, type, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList)),
			"Failed to Create Commnd List");
		m_CommandList->SetName(L"Command List");
		m_CommandList->Close();
	}

	void DirectX12CommandList::StartRecording()
	{
		CREATE_PROFILE_FUNCTIONI();
		CORE_ASSERT_HRESULT(m_CommandAllocator->Reset(), "Failed to reset command allocator");
		CORE_ASSERT_HRESULT(m_CommandList->Reset(m_CommandAllocator, nullptr), "Failed to reset command list");

		ID3D12DescriptorHeap* heaps[]{ DirectX12ResourceManager::s_SRVHeap->GetHeap().Get(), DirectX12ResourceManager::s_SamplerHeap->GetHeap().Get() };
		m_CommandList->SetDescriptorHeaps(2, heaps);

		// start tracy trace
#define BEGIN_GPU_TRACE(name) CREATE_SOURCE_LOC(name); BeginGPUEvent(&SOURCE_LOC_NAME);
		BEGIN_GPU_TRACE("CommandList");
#undef BEGIN_GPU_TRACE
		
	}

	void DirectX12CommandList::RecoredCommands(CPUCommandAllocator* commandAllocator)
	{
		CREATE_PROFILE_FUNCTIONI();
		for (uint32 i = 0; i < commandAllocator->GetCommands().Count(); i++)
		{
			CPUCommand* cmd = commandAllocator->GetCommands()[i];
#define CALL_COMMAND(obj, func) case obj::GetStaticCommandID(): func(*(obj*)cmd); continue

			// event tracking
			switch (cmd->GetCommandID())
			{
			case CPUBeginEventStaticCommand::GetStaticCommandID(): BeginEvent(((CPUBeginEventStaticCommand*)cmd)->eventName); continue;
			case CPUBeginEventDynamicCommand::GetStaticCommandID(): BeginEvent(((CPUBeginEventDynamicCommand*)cmd)->eventName.c_str()); continue;
			case CPUEndEventCommand::GetStaticCommandID(): EndEvent(); continue;
			case CPUBeginGPUEventCommand::GetStaticCommandID(): BeginGPUEvent(((CPUBeginGPUEventCommand*)cmd)->data); continue;
			case CPUEndGPUEventCommand::GetStaticCommandID(): EndGPUEvent(); continue;
			default: break;
			}

			// state management
			switch (cmd->GetCommandID())
			{
				CALL_COMMAND(CPUResourceTransitionCommand, Transition);
				CALL_COMMAND(CPUAwaitUAVCommand, AwaitUAVs);
				CALL_COMMAND(CPUOpenTransientCommand, OpenTransient);
				CALL_COMMAND(CPUCloseTransientCommand, CloseTransient);
			default: break;
			}

			FlushResourceBarriers();

			// commands
			switch (cmd->GetCommandID())
			{
				CALL_COMMAND(CPUResolveMSAACommand, ResolveMSAA);
				CALL_COMMAND(CPUCopyBufferCommand, CopyBuffer);
				CALL_COMMAND(CPUUploadTextureCommand , UploadTexture);
				CALL_COMMAND(CPUSetViewportCommand, SetViewport);
				CALL_COMMAND(CPUSetRenderTargetCommand, SetRenderTarget);
				CALL_COMMAND(CPUClearRenderTargetCommand, ClearRenderTarget);
				CALL_COMMAND(CPUSetGraphicsShaderCommand, SetShader);
				CALL_COMMAND(CPUSetComputeShaderCommand, SetShader);
				CALL_COMMAND(CPUSetWorkGraphShaderCommand, SetShader);
				CALL_COMMAND(CPUBindDataCommand, BindData);
				CALL_COMMAND(CPUDrawMeshCommand, DrawMesh);
				CALL_COMMAND(CPUDispatchCommand, Dispatch);
				CALL_COMMAND(CPUDispatchGraphCPUDataCommand, DispatchGraph);
				CALL_COMMAND(CPUDispatchGraphGPUDataCommand, DispatchGraph);
			default: break;
			}
#undef CALL_COMMAND
		}
	}

	void DirectX12CommandList::Close()
	{
		CREATE_PROFILE_FUNCTIONI();

		FlushResourceBarriers();

		// end tracy trace
		EndGPUEvent();

		m_CommandList->Close();
	}


	void DirectX12CommandList::Transition(const Utils::Vector<ResourceTransitionObject>& transitions)
	{
		CPUResourceTransitionCommand cmd;
		cmd.resourceStateTransitons = transitions;
		Transition(cmd);
	}

	void DirectX12CommandList::SetRenderTarget(Ref<RenderTarget2D> renderTarget)
	{
		CPUSetRenderTargetCommand cmd;
		cmd.renderTargetHandles.Push(renderTarget->GetRTVDSVDescriptor());
		cmd.depthStencil = nullptr;
		SetRenderTarget(cmd);
	}

	void DirectX12CommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget, Math::Vector4 color)
	{
		CPUClearRenderTargetCommand cmd;
		cmd.handle = renderTarget->GetRTVDSVDescriptor();
		cmd.color = color;
		cmd.isDepthStencil = false;
		ClearRenderTarget(cmd);
	}




	void DirectX12CommandList::FlushResourceBarriers()
	{
		if (m_ResourceBarriers.Empty())
			return;

		m_CommandList->ResourceBarrier((uint32)m_ResourceBarriers.Count(), m_ResourceBarriers.Data());

		m_ResourceBarriers.Clear();
	}

	void DirectX12CommandList::BeginEvent(const char* eventName)
	{
		GPUTimer::BeginEvent(this, eventName);
		m_EventStack.Push(eventName);
	}

	void DirectX12CommandList::EndEvent()
	{
		GPUTimer::EndEvent(this);
		m_EventStack.Pop();
	}

	void DirectX12CommandList::BeginGPUEvent(const tracy::SourceLocationData* data)
	{
		TracyD3D12Ctx ctx = Renderer::GetMainCommandQueue<DirectX12CommandQueue>()->GetTracyCtx();
		tracy::D3D12ZoneScope* zone = new tracy::D3D12ZoneScope( ctx, m_CommandList, data, true );
		m_TracyEventStack.Push(zone);
	}

	void DirectX12CommandList::EndGPUEvent()
	{
		tracy::D3D12ZoneScope* zone = m_TracyEventStack.Pop();
		delete zone;
	}

	// transitions
	void DirectX12CommandList::Transition(const CPUResourceTransitionCommand& cmd)
	{
		const Utils::Vector<ResourceTransitionObject>& transitions = cmd.resourceStateTransitons;

		if (transitions.Count() == 0)
			return;


		for (uint32 i = 0; i < transitions.Count(); i++)
		{
			ResourceState to = transitions[i].to;
			ResourceState from = transitions[i].from;
			GPUResource* resource = transitions[i].resource;
			D3D12_RESOURCE_BARRIER b = TransitionResource(
				(ID3D12Resource*)resource->GetGPUResourcePointer(),
				(D3D12_RESOURCE_STATES)resource->GetGPUState(from), 
				(D3D12_RESOURCE_STATES)resource->GetGPUState(to)
			);

			m_ResourceBarriers.Push(b);
		}
	}

	// UAVs
	void DirectX12CommandList::AwaitUAVs(const CPUAwaitUAVCommand& cmd)
	{
		if (cmd.UAVs.Count() == 0)
			return;

		for (uint32 i = 0; i < cmd.UAVs.Count(); i++)
		{
			CD3DX12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::UAV((ID3D12Resource*)cmd.UAVs[i]->GetGPUResourcePointer());
			m_ResourceBarriers.Push(b);
		}
	}

	// transient allocation
	void DirectX12CommandList::OpenTransient(const CPUOpenTransientCommand& cmd)
	{
		CD3DX12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Aliasing(nullptr, (ID3D12Resource*)cmd.res->GetGPUResourcePointer());
		m_ResourceBarriers.Push(b);
	}

	void DirectX12CommandList::CloseTransient(const CPUCloseTransientCommand& cmd)
	{
		// nothing to do
	}

	void DirectX12CommandList::ResolveMSAA(const CPUResolveMSAACommand& cmd)
	{
		for (uint32 i = 0; i < cmd.resolves.Count(); i++)
		{
			DirectX12Texture2DResource* dest = (DirectX12Texture2DResource*)cmd.resolves[i].dest;
			DirectX12Texture2DResource* src = (DirectX12Texture2DResource*)cmd.resolves[i].src;

			m_CommandList->ResolveSubresource(dest->GetBuffer(), 0, src->GetBuffer(), 0, dest->GetDXGISRVFormat());
		}
	}

	// copying
	void DirectX12CommandList::CopyBuffer(const CPUCopyBufferCommand& cmd)
	{
		ID3D12Resource* destp = (ID3D12Resource*)cmd.dest->GetGPUResourcePointer();
		ID3D12Resource* srcp = (ID3D12Resource*)cmd.src->GetGPUResourcePointer();
		if (cmd.size == 0)
			m_CommandList->CopyResource(destp, srcp);
		else
			m_CommandList->CopyBufferRegion(destp, cmd.destOffset, srcp, cmd.srcOffset, cmd.size);
	}

	void DirectX12CommandList::UploadTexture(const CPUUploadTextureCommand& cmd)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = (ID3D12Resource*)cmd.src->GetGPUResourcePointer();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Format = GetDXGITextureFormat(cmd.src->GetFormat());
		srcLocation.PlacedFootprint.Footprint.Width = cmd.src->GetWidth();
		srcLocation.PlacedFootprint.Footprint.Height = cmd.src->GetHeight();
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = cmd.src->GetPitch();

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = (ID3D12Resource*)cmd.dest->GetGPUResourcePointer();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;
		m_CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	// set render targets and viewport
	void DirectX12CommandList::SetViewport(const CPUSetViewportCommand& cmd)
	{
		const D3D12_VIEWPORT viewport{
			cmd.pos.x,
			cmd.pos.y,
			cmd.size.x,
			cmd.size.y,
			cmd.depths.x,
			cmd.depths.y
		};

		const D3D12_RECT rect{
			(long)cmd.pos.x,
			(long)cmd.pos.y,
			(long)cmd.size.x,
			(long)cmd.size.y,
		};

		m_CommandList->RSSetViewports(1, &viewport);
		m_CommandList->RSSetScissorRects(1, &rect);
	}

	void DirectX12CommandList::SetRenderTarget(const CPUSetRenderTargetCommand& cmd)
	{
		// get render target information
		Utils::Vector<uint64> rendertargetHandles(cmd.renderTargetHandles.Count());
		uint64 depthHandle = 0;

		for (uint32 i = 0; i < cmd.renderTargetHandles.Count(); i++)
		{
			DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)cmd.renderTargetHandles[i];
			rendertargetHandles[i] = rtv->GetHandle().cpu.ptr;
		}

		if (cmd.depthStencil != nullptr)
		{
			DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)cmd.depthStencil;
			depthHandle = rtv->GetHandle().cpu.ptr;
		}

		// set render target
		m_CommandList->OMSetRenderTargets(
			(uint32)rendertargetHandles.Count(),
			(D3D12_CPU_DESCRIPTOR_HANDLE*)rendertargetHandles.Data(),
			FALSE,
			depthHandle ? (D3D12_CPU_DESCRIPTOR_HANDLE*)&depthHandle : nullptr
		);
	}

	// clear render targets
	void DirectX12CommandList::ClearRenderTarget(const CPUClearRenderTargetCommand& cmd)
	{
		DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)cmd.handle;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv->GetHandle().cpu;
		if (cmd.isDepthStencil)
			m_CommandList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, cmd.color.r, (uint8)cmd.color.g, 0, nullptr);
		else
			m_CommandList->ClearRenderTargetView(handle, (float*)&cmd.color, 0, nullptr);

	}

	// set shaders
	void DirectX12CommandList::InitalizeDescriptorTables(const Ref<ShaderPass> shader)
	{
		Utils::Vector<ShaderParameter> reflectionData = shader->GetReflectionData();
		for (uint32 i = 0; i < reflectionData.Count(); i++)
		{
			ShaderParameter& p = reflectionData[i];
			if (p.type == PerameterType::DescriptorTable && p.count > 1)
			{
				switch (p.descType)
				{
				case DescriptorType::CBV:
				case DescriptorType::SRV:
				case DescriptorType::UAV:
					if(shader->IsComputeShader())
						m_CommandList->SetComputeRootDescriptorTable(p.rootIndex, DirectX12ResourceManager::s_SRVHeap->GetGPUHeapStart());
					else
						m_CommandList->SetGraphicsRootDescriptorTable(p.rootIndex, DirectX12ResourceManager::s_SRVHeap->GetGPUHeapStart());
					break;
				default:
					break;
				}
			}
		}
	}

	void DirectX12CommandList::SetShader(const CPUSetGraphicsShaderCommand& cmd)
	{
		Ref<DirectX12GraphicsShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12GraphicsShaderPass>(cmd.shaderPass);
		ID3D12PipelineState* pipline = dxShader->GetPipelineState(cmd.fbDesc);
		m_CommandList->SetPipelineState(pipline);

		D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (dxShader->GetTopologyType())
		{
		case Topology::Triangle:	top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		case Topology::Line:		top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
		case Topology::Point:		top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
		}

		m_CommandList->SetGraphicsRootSignature(dxShader->GetRootSignature());
		m_CommandList->IASetPrimitiveTopology(top);

		InitalizeDescriptorTables(cmd.shaderPass);
	}

	void DirectX12CommandList::SetShader(const CPUSetComputeShaderCommand& cmd)
	{
		Ref<DirectX12ComputeShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12ComputeShaderPass>(cmd.shaderPass);
		m_CommandList->SetPipelineState(dxShader->GetPipelineState());
		m_CommandList->SetComputeRootSignature(dxShader->GetRootSignature());

		InitalizeDescriptorTables(cmd.shaderPass);
	}

	void DirectX12CommandList::SetShader(const CPUSetWorkGraphShaderCommand& cmd)
	{
		Ref<DirectX12WorkGraphShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12WorkGraphShaderPass>(cmd.shaderPass);
		m_CommandList->SetComputeRootSignature(dxShader->GetRootSignature());

		D3D12_SET_PROGRAM_DESC setProg = {};
		setProg.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
		setProg.WorkGraph.ProgramIdentifier = dxShader->GetIdentifier();
		setProg.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
		setProg.WorkGraph.BackingMemory = dxShader->GetBackingMemory();
		m_CommandList->SetProgram(&setProg);

		InitalizeDescriptorTables(cmd.shaderPass);
	}


	// set data
	void DirectX12CommandList::BindData(const CPUBindDataCommand& cmd)
	{
		for (uint32 i = 0; i < cmd.bindings.Count(); i++)
		{
			const GPUDataBinding& binding = cmd.bindings[i];
			switch (binding.type)
			{
#define CALL_BINDING(type, func) case GPUDataBinding::Type::type: func(binding); break
				CALL_BINDING(RootConstant, SetRootConstant);
				CALL_BINDING(ConstantBuffer, SetConstantBuffer);
				CALL_BINDING(StructuredBuffer, SetStructuredBuffer);
				CALL_BINDING(RWStructuredBuffer, SetRWStructuredBuffer);
				CALL_BINDING(Texture2D, SetTexture);
				CALL_BINDING(RWTexture2D, SetRWTexture);
			default:
				CORE_WARN("Unrecognized binding type");
				break;
#undef CALL_BINDING
			}
		}
	}

	void DirectX12CommandList::SetRootConstant(const GPUDataBinding& binding)
	{
		if (binding.isCompute)
			m_CommandList->SetComputeRoot32BitConstant(binding.index, binding.data, 0);
		else
			m_CommandList->SetGraphicsRoot32BitConstant(binding.index, binding.data, 0);
	}

	void DirectX12CommandList::SetConstantBuffer(const GPUDataBinding& binding)
	{
		DirectX12ConstantBufferResource* dxResource = (DirectX12ConstantBufferResource*)binding.data;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dxResource->GetBuffer()->GetGPUVirtualAddress();
		if (binding.isCompute)
			m_CommandList->SetComputeRootConstantBufferView(binding.index, gpuAddress);
		else
			m_CommandList->SetGraphicsRootConstantBufferView(binding.index, gpuAddress);
	}

	void DirectX12CommandList::SetStructuredBuffer(const GPUDataBinding& binding)
	{
		DirectX12StructuredBufferSRVDescriptorHandle* srv = (DirectX12StructuredBufferSRVDescriptorHandle*)binding.data;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;
		if (binding.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(binding.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(binding.index, gpuHandle);
	}

	void DirectX12CommandList::SetRWStructuredBuffer(const GPUDataBinding& binding)
	{
		DirectX12StructuredBufferUAVDescriptorHandle* uav = (DirectX12StructuredBufferUAVDescriptorHandle*)binding.data;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = uav->GetHandle().gpu;

		if (binding.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(binding.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(binding.index, gpuHandle);
	}

	void DirectX12CommandList::SetTexture(const GPUDataBinding& binding)
	{

		DirectX12Texture2DSRVDescriptorHandle* srv = (DirectX12Texture2DSRVDescriptorHandle*)binding.data;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;

		if (binding.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(binding.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(binding.index, gpuHandle);
	}

	void DirectX12CommandList::SetRWTexture(const GPUDataBinding& binding)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = ((DirectX12Texture2DUAVDescriptorHandle*)binding.data)->GetHandle().gpu;

		if (binding.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(binding.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(binding.index, gpuHandle);
	}

	void DirectX12CommandList::DrawMesh(const CPUDrawMeshCommand& cmd)
	{
		DirectX12IndexBufferView* indexBufferView = (DirectX12IndexBufferView*)cmd.indexBufferView;

		Utils::Vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews;
		vertexBufferViews.Resize(cmd.vertexBufferViews.Count());
		for (uint32 i =0; i < cmd.vertexBufferViews.Count(); i++)
		{
			DirectX12VertexBufferView* vb = (DirectX12VertexBufferView*)cmd.vertexBufferViews[i];
			vertexBufferViews[i] = vb->GetView();
		}

		m_CommandList->IASetVertexBuffers(0, vertexBufferViews.Count(), vertexBufferViews.Data());
		m_CommandList->IASetIndexBuffer(&indexBufferView->GetView());
		m_CommandList->DrawIndexedInstanced(cmd.numIndices, cmd.numInstances, 0, 0, 0);
	}

	void DirectX12CommandList::Dispatch(const CPUDispatchCommand& cmd)
	{
		m_CommandList->Dispatch(cmd.threadGroupsX, cmd.threadGroupsY, cmd.threadGroupsZ);
	}

	void DirectX12CommandList::DispatchGraph(const CPUDispatchGraphCPUDataCommand& cmd)
	{
		D3D12_DISPATCH_GRAPH_DESC DSDesc = {};
		DSDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
		DSDesc.NodeCPUInput.EntrypointIndex = 0; // just one entrypoint in this graph
		DSDesc.NodeCPUInput.NumRecords = cmd.count;
		DSDesc.NodeCPUInput.RecordStrideInBytes = cmd.stride;
		DSDesc.NodeCPUInput.pRecords = cmd.data.Data();
		m_CommandList->DispatchGraph(&DSDesc);
	}

	void DirectX12CommandList::DispatchGraph(const CPUDispatchGraphGPUDataCommand& cmd)
	{
		DirectX12StructuredBufferResource* res = (DirectX12StructuredBufferResource*)cmd.res;

		D3D12_DISPATCH_GRAPH_DESC DSDesc = {};
		DSDesc.Mode = D3D12_DISPATCH_MODE_NODE_GPU_INPUT;
		DSDesc.NodeGPUInput = res->GetBuffer()->GetGPUVirtualAddress();
		m_CommandList->DispatchGraph(&DSDesc);
	}
}
