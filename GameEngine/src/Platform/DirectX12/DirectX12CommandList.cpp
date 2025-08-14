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
#include "Resources/DirectX12InstanceBuffer.h"
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
	}

	void DirectX12CommandList::RecoredCommands(CPUCommandAllocator* commandAllocator)
	{
		CREATE_PROFILE_FUNCTIONI();
		for (uint32 i = 0; i < commandAllocator->GetCommands().Count(); i++)
		{
			CPUCommand* cmd = commandAllocator->GetCommands()[i];
			switch (cmd->GetCommandID())
			{
#define CALL_COMMAND(obj, func) case obj::GetStaticCommandID(): func(*(obj*)cmd); break
			case CPUEndEventCommand::GetStaticCommandID(): EndEvent(); break;
			case CPUBeginEventStaticCommand::GetStaticCommandID(): BeginEvent(((CPUBeginEventStaticCommand*)cmd)->eventName); break;
			case CPUBeginEventDynamicCommand::GetStaticCommandID(): BeginEvent(((CPUBeginEventDynamicCommand*)cmd)->eventName.c_str()); break;
				CALL_COMMAND(CPUResourceTransitionCommand, Transition);
				CALL_COMMAND(CPUAwaitUAVCommand, AwaitUAVs);
				CALL_COMMAND(CPUCopyBufferCommand, CopyBuffer);
				CALL_COMMAND(CPUUploadTextureCommand , UploadTexture);
				CALL_COMMAND(CPUSetViewportCommand, SetViewport);
				CALL_COMMAND(CPUSetRenderTargetCommand, SetRenderTarget);
				CALL_COMMAND(CPUClearRenderTargetCommand, ClearRenderTarget);
				CALL_COMMAND(CPUSetGraphicsShaderCommand, SetShader);
				CALL_COMMAND(CPUSetComputeShaderCommand, SetShader);
				CALL_COMMAND(CPUSetWorkGraphShaderCommand, SetShader);
				CALL_COMMAND(CPUSetRootConstantCommand, SetRootConstant);
				CALL_COMMAND(CPUSetConstantBufferCommand, SetConstantBuffer);
				CALL_COMMAND(CPUSetStructuredBufferCommand, SetStructuredBuffer);
				CALL_COMMAND(CPUSetRWStructuredBufferCommand, SetRWStructuredBuffer);
				CALL_COMMAND(CPUSetTextureCommand, SetTexture);
				CALL_COMMAND(CPUSetRWTextureCommand, SetRWTexture);
				CALL_COMMAND(CPUDrawMeshCommand, DrawMesh);
				CALL_COMMAND(CPUDispatchCommand, Dispatch);
				CALL_COMMAND(CPUDispatchGraphCPUDataCommand, DispatchGraph);
				CALL_COMMAND(CPUDispatchGraphGPUDataCommand, DispatchGraph);
#undef CALL_COMMAND
			default:
				break;
			}
		}
	}

	void DirectX12CommandList::Close()
	{
		CREATE_PROFILE_FUNCTIONI();
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

	// transitions
	void DirectX12CommandList::Transition(const CPUResourceTransitionCommand& cmd)
	{
		const Utils::Vector<ResourceTransitionObject>& transitions = cmd.resourceStateTransitons;

		if (transitions.Count() == 0)
			return;

		Utils::Vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.Resize(transitions.Count());
		for (uint32 i = 0; i < transitions.Count(); i++)
		{
			ResourceState to = transitions[i].to;
			ResourceState from = transitions[i].from;
			GPUResource* resource = transitions[i].resource;
			barriers[i] = TransitionResource((ID3D12Resource*)resource->GetGPUResourcePointer(), 
				(D3D12_RESOURCE_STATES)resource->GetGPUState(from), (D3D12_RESOURCE_STATES)resource->GetGPUState(to));
		}

		m_CommandList->ResourceBarrier((uint32)barriers.Count(), barriers.Data());
	}

	// UAVs
	void DirectX12CommandList::AwaitUAVs(const CPUAwaitUAVCommand& cmd)
	{
		if (cmd.UAVs.Count() == 0)
			return;

		Utils::Vector<CD3DX12_RESOURCE_BARRIER> barriers;
		barriers.Resize(cmd.UAVs.Count());
		for (uint32 i = 0; i < cmd.UAVs.Count(); i++)
			barriers[i] = CD3DX12_RESOURCE_BARRIER::UAV((ID3D12Resource*)cmd.UAVs[i]->GetGPUResourcePointer());
		m_CommandList->ResourceBarrier(barriers.Count(), barriers.Data());
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

	void DISABLE_OPS DirectX12CommandList::SetShader(const CPUSetGraphicsShaderCommand& cmd)
	{
		Ref<DirectX12GraphicsShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12GraphicsShaderPass>(cmd.shaderPass);
		ID3D12PipelineState* pipline = dxShader->GetPipelineState(cmd.format);
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
	void DirectX12CommandList::SetRootConstant(const CPUSetRootConstantCommand& cmd)
	{
		if (cmd.isCompute)
			m_CommandList->SetComputeRoot32BitConstant(cmd.index, cmd.data, 0);
		else
			m_CommandList->SetGraphicsRoot32BitConstant(cmd.index, cmd.data, 0);
	}

	void DirectX12CommandList::SetConstantBuffer(const CPUSetConstantBufferCommand& cmd)
	{
		DirectX12ConstantBufferResource* dxResource = (DirectX12ConstantBufferResource*)cmd.res;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dxResource->GetBuffer()->GetGPUVirtualAddress();
		if (cmd.isCompute)
			m_CommandList->SetComputeRootConstantBufferView(cmd.index, gpuAddress);
		else
			m_CommandList->SetGraphicsRootConstantBufferView(cmd.index, gpuAddress);
	}

	void DirectX12CommandList::SetStructuredBuffer(const CPUSetStructuredBufferCommand& cmd)
	{
		DirectX12StructuredBufferSRVDescriptorHandle* srv = (DirectX12StructuredBufferSRVDescriptorHandle*)cmd.handle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;
		if (cmd.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(cmd.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(cmd.index, gpuHandle);
	}

	void DirectX12CommandList::SetRWStructuredBuffer(const CPUSetRWStructuredBufferCommand& cmd)
	{
		DirectX12StructuredBufferUAVDescriptorHandle* uav = (DirectX12StructuredBufferUAVDescriptorHandle*)cmd.handle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = uav->GetHandle().gpu;

		if (cmd.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(cmd.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(cmd.index, gpuHandle);
	}

	void DirectX12CommandList::SetTexture(const CPUSetTextureCommand& cmd)
	{

		DirectX12Texture2DSRVDescriptorHandle* srv = (DirectX12Texture2DSRVDescriptorHandle*)cmd.handle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;

		if (cmd.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(cmd.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(cmd.index, gpuHandle);
	}

	void DirectX12CommandList::SetRWTexture(const CPUSetRWTextureCommand& cmd)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = ((DirectX12Texture2DUAVDescriptorHandle*)cmd.handle)->GetHandle().gpu;

		if (cmd.isCompute)
			m_CommandList->SetComputeRootDescriptorTable(cmd.index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(cmd.index, gpuHandle);
	}

	void DirectX12CommandList::DrawMesh(const CPUDrawMeshCommand& cmd)
	{
		DirectX12VertexBufferView* vertexBufferView = (DirectX12VertexBufferView*)cmd.vertexBufferView;
		DirectX12IndexBufferView* indexBufferView = (DirectX12IndexBufferView*)cmd.indexBufferView;

		if (cmd.instanceBufferView)
		{
			DirectX12InstanceBufferView* instanceBufferView = (DirectX12InstanceBufferView*)cmd.instanceBufferView;

			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView(), instanceBufferView->GetView() };
			m_CommandList->IASetVertexBuffers(0, 2, views);
			m_CommandList->IASetIndexBuffer(&indexBufferView->GetView());
			m_CommandList->DrawIndexedInstanced(cmd.numIndices, cmd.numInstances, 0, 0, 0);
		}
		else
		{
			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView() };
			m_CommandList->IASetVertexBuffers(0, 1, views);
			m_CommandList->IASetIndexBuffer(&indexBufferView->GetView());
			m_CommandList->DrawIndexedInstanced(cmd.numIndices, 1, 0, 0, 0);
		}
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
