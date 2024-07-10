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

#include "Engine/Util/Performance.h"
#include "Engine/Renderer/Mesh.h"

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

	static inline D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandList::CommandListType type)
	{
		switch (type)
		{
		case Engine::CommandList::Direct: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case Engine::CommandList::Bundle: return D3D12_COMMAND_LIST_TYPE_BUNDLE;
		}
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}

	void DirectX12CommandList::Init()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		const uint32 numAllocators = 2;
		m_Frames.resize(numAllocators);
		D3D12_COMMAND_LIST_TYPE type = GetD3D12CommandListType(m_Type);
		for (uint32 i = 0; i < numAllocators; i++)
		{
			CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_Frames[i].commandAllocator)),
				"Failed to Create Command List Allocator");
			m_Frames[i].commandAllocator->SetName(L"Command Allocator");
		}
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandList(0, type, m_Frames[0].commandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList)),
			"Failed to Create Commnd List");
		m_CommandList->SetName(L"Command List");
		m_CommandList->Close();

		
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_PrependAllocator)),
			"Failed to Create Prepend Command List Allocator");
		m_PrependAllocator->SetName(L"Prepend Allocator");
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandList(0,type, m_PrependAllocator, nullptr, IID_PPV_ARGS(&m_PrependList)),
			"Failed to Create Prepend Command List");
		m_PrependList->SetName(L"Prepend Command List");
		m_PrependList->Close();

		m_RecordFlag.Signal();
	}

	void DirectX12CommandList::InternalClose()
	{
		// TODO
		if (m_State != RecordState::Closed)
		{
			m_CommandList->Close();
			m_State = RecordState::Closed;
		}
	}

	void DirectX12CommandList::StartRecording()
	{
		m_RecordFlag.Wait();
		m_RecordFlag.Clear();
		m_RenderTarget = nullptr;
		m_State = RecordState::Open;
		wrl::ComPtr<ID3D12CommandAllocator> allocator = GetAllocator();
		CORE_ASSERT_HRESULT(allocator->Reset(), "Failed to reset command allocator");
		CORE_ASSERT_HRESULT(m_CommandList->Reset(allocator.Get(), nullptr), "Failed to reset command list");

		ID3D12DescriptorHeap* heaps[]{ DirectX12ResourceManager::s_SRVHeap->GetHeap().Get(), DirectX12ResourceManager::s_SamplerHeap->GetHeap().Get() };
		m_CommandList->SetDescriptorHeaps(2, heaps);

		GetCurrentPendingTransitions().clear();
		GetResourceStates().clear();
	}


	// transitions

	void DirectX12CommandList::Present(Ref<FrameBuffer> fb)
	{
		if (fb == nullptr)
			fb = m_RenderTarget;

		std::vector<ResourceStateObject> transitions(fb->GetAttachments().size());
		for (uint32 i = 0; i < fb->GetAttachments().size(); i++)
			transitions[i] = { fb->GetAttachment(i)->GetResource(), ResourceState::Common };

		ValidateStates(transitions);

		if (fb == m_RenderTarget)
			m_RenderTarget = nullptr;
	}


	void DirectX12CommandList::Transition(std::vector<ResourceTransitionObject> transitions)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.resize(transitions.size());
		for (uint32 i = 0; i < transitions.size(); i++)
		{
			ResourceState to = transitions[i].to;
			ResourceState from = transitions[i].from;
			GPUResource* resource = transitions[i].resource;
			CORE_ASSERT(resource->SupportState(to), "resouce does not support state");
			barriers[i] = TransitionResource((ID3D12Resource*)resource->GetGPUResourcePointer(), 
				(D3D12_RESOURCE_STATES)resource->GetState(from), (D3D12_RESOURCE_STATES)resource->GetState(to));
		}

		m_CommandList->ResourceBarrier((uint32)barriers.size(), barriers.data());
	}

	void DirectX12CommandList::ValidateStates(std::vector<ResourceStateObject> resources)
	{
		std::vector<ResourceStateObject>& pendingTransitions = GetCurrentPendingTransitions();
		std::unordered_map<GPUResource*, ResourceState>& resourceStates = GetResourceStates();

		std::vector<ResourceTransitionObject> transitions;
		transitions.reserve(resources.size());

		for (ResourceStateObject& res : resources)
		{
			const std::unordered_map<GPUResource*, ResourceState>::iterator& foundResouce = resourceStates.find(res.resource);

			if (foundResouce == resourceStates.end())
			{
				pendingTransitions.push_back(res);
				resourceStates[res.resource] = res.state;
			}
			else
			{
				ResourceState currState = foundResouce->second;
				if (currState != res.state)
				{
					transitions.push_back({ res.resource, res.state, currState });
					resourceStates[res.resource] = res.state;
				}
			}
		}

		if (!transitions.empty())
			Transition(transitions);
	}

	// copying
	void DirectX12CommandList::CopyBuffer(GPUResource* dest, uint64 destOffset, GPUResource* src, uint64 srcOffset, uint64 size)
	{
		ID3D12Resource* destp = (ID3D12Resource*)dest->GetGPUResourcePointer();
		ID3D12Resource* srcp = (ID3D12Resource*)src->GetGPUResourcePointer();
		m_CommandList->CopyBufferRegion(destp, destOffset, srcp, srcOffset, size);
	}

	void DirectX12CommandList::CopyResource(GPUResource* dest, GPUResource* src)
	{
		m_CommandList->CopyResource((ID3D12Resource*)dest->GetGPUResourcePointer(), (ID3D12Resource*)src->GetGPUResourcePointer());
	}

	void DirectX12CommandList::UploadTexture(GPUResource* dest, UploadTextureResource* src)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = (ID3D12Resource*)src->GetGPUResourcePointer();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Format = GetDXGITextureFormat(src->GetFormat());
		srcLocation.PlacedFootprint.Footprint.Width = src->GetWidth();
		srcLocation.PlacedFootprint.Footprint.Height = src->GetHeight();
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = src->GetPitch();

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = (ID3D12Resource*)dest->GetGPUResourcePointer();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;
		m_CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	// rendering

	void DirectX12CommandList::SetRenderTarget(Ref<RenderTarget2D> renderTarget)
	{
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not set render targets. command will be ignored");
			return;
		}

		m_RenderTarget = FrameBuffer::Create({ renderTarget });

		ValidateState(renderTarget->GetResource(), ResourceState::RenderTarget);

		DirectX12Texture2DResource* res = (DirectX12Texture2DResource*)renderTarget->GetResource();
		DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)renderTarget->GetRTVDSVDescriptor();

		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = rtv->GetHandle().cpu;
		m_CommandList->OMSetRenderTargets(1, &cpuDescriptorHandle, FALSE, nullptr);

		uint32 width = res->GetWidth();
		uint32 height = res->GetHeight();

		const D3D12_VIEWPORT viewport{
			0,0,
			(float)width,
			(float)height,
			0,1
		};

		const D3D12_RECT rect{
			0,0,
			(long)width,
			(long)height
		};

		m_CommandList->RSSetViewports(1, &viewport);
		m_CommandList->RSSetScissorRects(1, &rect);
	}

	void DirectX12CommandList::SetRenderTarget(Ref<FrameBuffer> buffer)
	{
		// check if command list is a bundle
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not set render targets. command will be ignored");
			return;
		}

		// get directX 12 frame buffer
		m_RenderTarget = buffer;

		// get render target information
		uint32 numRenderTargets = buffer->GetAttachments().size() - (size_t)m_RenderTarget->HasDepthAttachment();
		std::vector<ResourceStateObject> resourceStateValidation;
		std::vector<uint64> rendertargetHandles(numRenderTargets);
		std::vector<D3D12_VIEWPORT> viewports(numRenderTargets);
		std::vector<D3D12_RECT> rects(numRenderTargets);
		uint64 depthHandle = 0;

		for (uint32 i = 0; i < buffer->GetAttachments().size(); i++)
		{
			resourceStateValidation.push_back({ buffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget });
			DirectX12Texture2DResource* res = (DirectX12Texture2DResource*)buffer->GetAttachment(i)->GetResource();
			DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)buffer->GetAttachment(i)->GetRTVDSVDescriptor();


			if (IsTextureFormatDepthStencil(buffer->GetAttachment(i)->GetResource()->GetFormat()))
				depthHandle = rtv->GetHandle().cpu.ptr;
			else
			{
				viewports[i] = {
					0,0,
					(float)res->GetWidth(),
					(float)res->GetHeight(),
					0, 1
				};

				rects[i] = {
					0,0,
					(long)res->GetWidth(),
					(long)res->GetHeight()
				};

				rendertargetHandles[i] = rtv->GetHandle().cpu.ptr;
			}
		}

		ValidateStates(resourceStateValidation);

		// set render target
		m_CommandList->OMSetRenderTargets(
			(uint32)numRenderTargets, 
			(D3D12_CPU_DESCRIPTOR_HANDLE*)rendertargetHandles.data(), 
			FALSE, 
			depthHandle ? (D3D12_CPU_DESCRIPTOR_HANDLE*)&depthHandle : nullptr
		);

		// set scissor and viewport
		m_CommandList->RSSetViewports(numRenderTargets, viewports.data());
		m_CommandList->RSSetScissorRects(numRenderTargets, rects.data());
	}

	void DirectX12CommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color)
	{
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not clear render tartets. command will be ignored");
			return;
		}

		DirectX12Texture2DResource* res = (DirectX12Texture2DResource*)renderTarget->GetResource();
		DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)renderTarget->GetRTVDSVDescriptor();

		ValidateState({ res, ResourceState::RenderTarget });

		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv->GetHandle().cpu;
		if (IsTextureFormatDepthStencil(res->GetFormat()))
			m_CommandList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, color.r, (uint8)color.g, 0, nullptr);
		else
			m_CommandList->ClearRenderTargetView(handle, (float*)&color, 0, nullptr);
	}

	void DirectX12CommandList::InitalizeDescriptorTables(Ref<ShaderPass> shader)
	{
		std::vector<ShaderParameter> reflectionData = shader->GetReflectionData();
		for (uint32 i = 0; i < reflectionData.size(); i++)
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

	void DirectX12CommandList::SetShader(Ref<GraphicsShaderPass> shader)
	{

		if (!m_RenderTarget)
		{
			CORE_ERROR("Can not set shader without rendertarget");
			return;
		}

		Ref<DirectX12GraphicsShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12GraphicsShaderPass>(shader);
		m_CommandList->SetPipelineState(dxShader->GetPipelineState(m_RenderTarget));

		D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (dxShader->GetTopologyType())
		{
		case Topology::Triangle:	top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		case Topology::Line:		top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
		case Topology::Point:		top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
		}

		m_CommandList->SetGraphicsRootSignature(dxShader->GetRootSignature());
		m_CommandList->IASetPrimitiveTopology(top);

		InitalizeDescriptorTables(shader);

		m_BoundShader = shader;
	}

	void DirectX12CommandList::SetShader(Ref<ComputeShaderPass> shader)
	{
		Ref<DirectX12ComputeShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12ComputeShaderPass>(shader);
		m_CommandList->SetPipelineState(dxShader->GetPipelineState());
		m_CommandList->SetComputeRootSignature(dxShader->GetRootSignature());

		InitalizeDescriptorTables(shader);

		m_BoundShader = shader;
	}

	void DirectX12CommandList::SetShader(Ref<WorkGraphShaderPass> shader)
	{
		Ref<DirectX12WorkGraphShaderPass> dxShader = std::dynamic_pointer_cast<DirectX12WorkGraphShaderPass>(shader);

		m_CommandList->SetComputeRootSignature(dxShader->GetRootSignature());

		D3D12_SET_PROGRAM_DESC setProg = {};
		setProg.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
		setProg.WorkGraph.ProgramIdentifier = dxShader->GetIdentifier();
		setProg.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
		setProg.WorkGraph.BackingMemory = dxShader->GetBackingMemory();
		m_CommandList->SetProgram(&setProg);

		InitalizeDescriptorTables(shader);
		m_BoundShader = shader;
	}

	void DirectX12CommandList::SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer)
	{
		if (index == UINT32_MAX || buffer == nullptr)
			return; // invalid bind slot

		DirectX12ConstantBufferResource* dxResource = (DirectX12ConstantBufferResource*)buffer->GetResource();
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dxResource->GetBuffer()->GetGPUVirtualAddress();
		if (m_BoundShader->IsComputeShader())
			m_CommandList->SetComputeRootConstantBufferView(index, gpuAddress);
		else
			m_CommandList->SetGraphicsRootConstantBufferView(index, gpuAddress);
	}

	void DirectX12CommandList::SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer)
	{
		if (index == UINT32_MAX || buffer == nullptr)
			return;

		DirectX12StructuredBufferSRVDescriptorHandle* srv = (DirectX12StructuredBufferSRVDescriptorHandle*)buffer->GetSRVDescriptor();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;
		if (m_BoundShader->IsComputeShader())
			m_CommandList->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	void DirectX12CommandList::SetRootConstant(uint32 index, uint32 data)
	{
		if (index == UINT32_MAX)
			return; // invalid bind slot

		if (m_BoundShader->IsComputeShader())
			m_CommandList->SetComputeRoot32BitConstant(index, data, 0);
		else
			m_CommandList->SetGraphicsRoot32BitConstant(index, data, 0);
	}

	void DirectX12CommandList::SetTexture(uint32 index, Ref<Texture2D> texture)
	{
		if (index == UINT32_MAX)
			return; // invalid bind slot

		ValidateState(texture->GetResource(), ResourceState::ShaderResource);

		DirectX12Texture2DSRVDescriptorHandle* srv = (DirectX12Texture2DSRVDescriptorHandle*)texture->GetSRVDescriptor();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;

		if (m_BoundShader->IsComputeShader())
			m_CommandList->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	void DirectX12CommandList::SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav)
	{
		if (index == UINT32_MAX)
			return; // invalid bind slot

		ValidateState(uav->m_Resource, ResourceState::UnorderedResource);

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = ((DirectX12Texture2DUAVDescriptorHandle*)uav)->GetHandle().gpu;

		if (m_BoundShader->IsComputeShader())
			m_CommandList->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			m_CommandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	void DirectX12CommandList::DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer, int numInstances)
	{
		uint32 indexCount = mesh->GetIndexBuffer()->GetResource()->GetCount();
		DirectX12VertexBufferView* vertexBufferView = (DirectX12VertexBufferView*)mesh->GetVertexBuffer()->GetView();
		DirectX12IndexBufferView* indexBufferView = (DirectX12IndexBufferView*)mesh->GetIndexBuffer()->GetView();

		if (instanceBuffer)
		{
			DirectX12InstanceBufferView* instanceBufferView = (DirectX12InstanceBufferView*)instanceBuffer->GetView();
			if (numInstances < 0)
				numInstances = instanceBuffer->GetCount();

			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView(), instanceBufferView->GetView() };
			m_CommandList->IASetVertexBuffers(0, 2, views);
			m_CommandList->IASetIndexBuffer(&indexBufferView->GetView());
			m_CommandList->DrawIndexedInstanced(indexCount, numInstances, 0, 0, 0);
		}
		else
		{
			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView() };
			m_CommandList->IASetVertexBuffers(0, 1, views);
			m_CommandList->IASetIndexBuffer(&indexBufferView->GetView());
			m_CommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
		}
	}

	void DirectX12CommandList::ExecuteBundle(Ref<CommandList> commandList)
	{
		Ref<DirectX12CommandList> dxCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(commandList);
		if (dxCommandList->m_Type != CommandList::Bundle)
			return;

		m_CommandList->ExecuteBundle(dxCommandList->GetCommandList());
	}

	void DirectX12CommandList::Dispatch(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGrouptsZ)
	{
		m_CommandList->Dispatch(threadGroupsX, threadGroupsY, threadGrouptsZ);
	}

	void DirectX12CommandList::DisbatchGraph(Ref<StructuredBuffer> buffer)
	{
		DirectX12StructuredBufferResource* res = (DirectX12StructuredBufferResource*)buffer->GetResource();

		D3D12_DISPATCH_GRAPH_DESC DSDesc = {};
		DSDesc.Mode = D3D12_DISPATCH_MODE_NODE_GPU_INPUT;
		DSDesc.NodeGPUInput = res->GetBuffer()->GetGPUVirtualAddress();
		m_CommandList->DispatchGraph(&DSDesc);
	}

	void DirectX12CommandList::DispatchGraph(void* data, uint32 stride, uint32 count)
	{
		D3D12_DISPATCH_GRAPH_DESC DSDesc = {};
		DSDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
		DSDesc.NodeCPUInput.EntrypointIndex = 0; // just one entrypoint in this graph
		DSDesc.NodeCPUInput.NumRecords = count;
		DSDesc.NodeCPUInput.RecordStrideInBytes = stride;
		DSDesc.NodeCPUInput.pRecords = data;
		m_CommandList->DispatchGraph(&DSDesc);
	}

	void DirectX12CommandList::AwaitUAV(GPUResource* uav)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV((ID3D12Resource*)uav->GetGPUResourcePointer());
		m_CommandList->ResourceBarrier(1, &barrier);
	}

	void DirectX12CommandList::Close()
	{
		m_State = RequestClose;
		m_RenderTarget = nullptr;
		m_CurrentFrame = (m_CurrentFrame + 1) % m_Frames.size();
	}

	bool DirectX12CommandList::RecordPrependCommands()
	{
		std::vector<ResourceStateObject>& pendingTransitions = GetPendingTransitions();
		std::unordered_map<GPUResource*, ResourceState> endingStates = GetEndingResourceStates();

		// get list off all resources that need to be changed
		std::vector<D3D12_RESOURCE_BARRIER> transitions;
		transitions.reserve(pendingTransitions.size());

		for (uint32 i = 0; i < pendingTransitions.size(); i++)
		{
			ResourceStateObject& rso = pendingTransitions[i];

			GPUResource* resource = rso.resource;

			ResourceState to = rso.state;
			ResourceState from = resource->GetDefultState();
			CORE_ASSERT(resource->SupportState(to), "resouce does not support state");
			if (rso.resource->GetDefultState() != rso.state)
			{
				ID3D12Resource* gpuResourcePointer = (ID3D12Resource*)resource->GetGPUResourcePointer();
				CORE_ASSERT(gpuResourcePointer != nullptr, "{0}", i);
				transitions.push_back(TransitionResource(gpuResourcePointer,
					(D3D12_RESOURCE_STATES)resource->GetState(from), (D3D12_RESOURCE_STATES)resource->GetState(to)));
			}
		}

		for (auto resState : endingStates)
			resState.first->m_DefultState = resState.second;

		if (!transitions.empty())
		{
			CORE_ASSERT_HRESULT(m_PrependAllocator->Reset(), "Failed to reset prepend command allocator");
			CORE_ASSERT_HRESULT(m_PrependList->Reset(m_PrependAllocator, nullptr), "Failed to reset prepend command list");

			ID3D12DescriptorHeap* heaps[]{ DirectX12ResourceManager::s_SRVHeap->GetHeap().Get(), DirectX12ResourceManager::s_SamplerHeap->GetHeap().Get() };
			m_PrependList->SetDescriptorHeaps(2, heaps);

			m_PrependList->ResourceBarrier((uint32)transitions.size(), transitions.data());


			m_PrependList->Close();

			return true;
		}
		return false;
	}

}
