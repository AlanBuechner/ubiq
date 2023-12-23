#include "pch.h"
#include "DirectX12Commands.h"

#include "Engine/Renderer/Commands/Commands.h"
#include "Engine/Renderer/Commands/CommandList.h"

#include "DirectX12CommandList.h"

#include "Platform/DirectX12/Resources/DirectX12Buffer.h"
#include "Platform/DirectX12/Resources/DirectX12InstanceBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12ConstantBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12StructuredBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12Texture.h"
#include "Platform/DirectX12/DirectX12Shader.h"

#include "Engine/Renderer/Resources/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Resources/ResourceState.h"

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

	void RecoardTransitionCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		TransitionCommand& transitionCommandData = (TransitionCommand&)commandData;
		
		std::vector<ResourceTransitionObject> transitions = transitionCommandData.m_Transitions;

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

		dxCmdList.GetCommandList()->ResourceBarrier((uint32)barriers.size(), barriers.data());
	}

	void RecoardSetRenderTargetCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetRenderTargetCommand& setRenderTargetCommandData = (SetRenderTargetCommand&)commandData;

		FrameBuffer* buffer = setRenderTargetCommandData.m_FrameBuffer;

		uint32 numRenderTargets = buffer->GetAttachments().size() - (size_t)buffer->HasDepthAttachment();
		std::vector<uint64> rendertargetHandles(numRenderTargets);
		std::vector<D3D12_VIEWPORT> viewports(numRenderTargets);
		std::vector<D3D12_RECT> rects(numRenderTargets);
		uint64 depthHandle = 0;

		for (uint32 i = 0; i < buffer->GetAttachments().size(); i++)
		{
			DirectX12Texture2DResource* res = (DirectX12Texture2DResource*)buffer->GetAttachment(i)->GetResource();
			DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)buffer->GetAttachment(i)->GetRTVDSVDescriptor();


			if (IsDepthStencil(buffer->GetAttachment(i)->GetResource()->GetFormat()))
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

		// set render target
		dxCmdList.GetCommandList()->OMSetRenderTargets(
			(uint32)numRenderTargets,
			(D3D12_CPU_DESCRIPTOR_HANDLE*)rendertargetHandles.data(),
			FALSE,
			depthHandle ? (D3D12_CPU_DESCRIPTOR_HANDLE*)&depthHandle : nullptr
		);

		// set scissor and viewport
		dxCmdList.GetCommandList()->RSSetViewports(numRenderTargets, viewports.data());
		dxCmdList.GetCommandList()->RSSetScissorRects(numRenderTargets, rects.data());
	}

	void RecoardClearRenderTargetCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		ClearRenderTargetCommand& clearRenderTargetCommandData = (ClearRenderTargetCommand&)commandData;

		RenderTarget2D* renderTarget = clearRenderTargetCommandData.m_RenderTarget;
		Math::Vector4 color = clearRenderTargetCommandData.m_Color;

		DirectX12Texture2DResource* res = (DirectX12Texture2DResource*)renderTarget->GetResource();
		DirectX12Texture2DRTVDSVDescriptorHandle* rtv = (DirectX12Texture2DRTVDSVDescriptorHandle*)renderTarget->GetRTVDSVDescriptor();

		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv->GetHandle().cpu;
		if (IsDepthStencil(res->GetFormat()))
			dxCmdList.GetCommandList()->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, color.r, (uint8)color.g, 0, nullptr);
		else
			dxCmdList.GetCommandList()->ClearRenderTargetView(handle, (float*)&color, 0, nullptr);
	}

	// set shader and shader data

	void RecoardSetShaderCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetShaderCommand& setShaderCommandData = (SetShaderCommand&)commandData;

		ShaderPass* shader = setShaderCommandData.m_ShaderPass;
		FrameBuffer* renderTarget = setShaderCommandData.m_BoundFrameBuffer;

		bool compute = shader->IsComputeShader();

		DirectX12Shader* dxShader = (DirectX12Shader*)shader;
		dxCmdList.GetCommandList()->SetPipelineState(dxShader->GetPipelineState(renderTarget).Get());
		if (compute)
			dxCmdList.GetCommandList()->SetComputeRootSignature(dxShader->GetRootSignature().Get());
		else
		{
			D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			switch (dxShader->GetTopologyType())
			{
			case ShaderConfig::Triangle:
				top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
			case ShaderConfig::Line:
				top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
			case ShaderConfig::Point:
				top = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
			}

			dxCmdList.GetCommandList()->SetGraphicsRootSignature(dxShader->GetRootSignature().Get());
			dxCmdList.GetCommandList()->IASetPrimitiveTopology(top);
		}

		std::vector<ShaderParameter> reflectionData = shader->GetReflectionData();
		for (uint32 i = 0; i < reflectionData.size(); i++)
		{
			ShaderParameter& p = reflectionData[i];
			if (p.type == ShaderParameter::PerameterType::DescriptorTable && p.count > 1)
			{
				switch (p.descType)
				{
				case ShaderParameter::DescriptorType::CBV:
				case ShaderParameter::DescriptorType::SRV:
				case ShaderParameter::DescriptorType::UAV:
					if (compute)
						dxCmdList.GetCommandList()->SetComputeRootDescriptorTable(p.rootIndex, DirectX12ResourceManager::s_SRVHeap->GetGPUHeapStart());
					else
						dxCmdList.GetCommandList()->SetGraphicsRootDescriptorTable(p.rootIndex, DirectX12ResourceManager::s_SRVHeap->GetGPUHeapStart());
					break;
				default:
					break;
				}
			}
		}
	}

	void RecoardSetRootConstantCommand(CommandList& cmdList, Command& commandData)
	{

		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetRootConstantCommand& setRootConstantCommandData = (SetRootConstantCommand&)commandData;

		bool isComputeShader = setRootConstantCommandData.m_IsComputeShader;
		uint32 index = setRootConstantCommandData.m_Index;
		uint32 data = setRootConstantCommandData.m_Data;

		if (isComputeShader)
			dxCmdList.GetCommandList()->SetComputeRoot32BitConstant(index, data, 0);
		else
			dxCmdList.GetCommandList()->SetGraphicsRoot32BitConstant(index, data, 0);
	}

	void RecoardSetConstantBufferCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetConstantBufferCommand& setConstantBufferCommandData = (SetConstantBufferCommand&)commandData;

		bool isComputeShader = setConstantBufferCommandData.m_IsComputeShader;
		uint32 index = setConstantBufferCommandData.m_Index;
		ConstantBuffer* buffer = setConstantBufferCommandData.m_Buffer;

		DirectX12ConstantBufferResource* dxResource = (DirectX12ConstantBufferResource*)buffer->GetResource();
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dxResource->GetBuffer()->GetGPUVirtualAddress();
		if (isComputeShader)
			dxCmdList.GetCommandList()->SetComputeRootConstantBufferView(index, gpuAddress);
		else
			dxCmdList.GetCommandList()->SetGraphicsRootConstantBufferView(index, gpuAddress);
	}

	void RecoardSetStructuredBufferCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetStructuredBufferCommand& setStructuredBufferCommandData = (SetStructuredBufferCommand&)commandData;

		bool isComputeShader = setStructuredBufferCommandData.m_IsComputeShader;
		uint32 index = setStructuredBufferCommandData.m_Index;
		StructuredBuffer* buffer = setStructuredBufferCommandData.m_Buffer;

		DirectX12StructuredBufferSRVDescriptorHandle* srv = (DirectX12StructuredBufferSRVDescriptorHandle*)buffer->GetSRVDescriptor();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;
		if (isComputeShader)
			dxCmdList.GetCommandList()->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			dxCmdList.GetCommandList()->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	void RecoardSetTextureCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetTextureCommand& setTextureCommandData = (SetTextureCommand&)commandData;

		bool isComputeShader = setTextureCommandData.m_IsComputeShader;
		uint32 index = setTextureCommandData.m_Index;
		Texture2D* texture = setTextureCommandData.m_Texture;

		DirectX12Texture2DSRVDescriptorHandle* srv = (DirectX12Texture2DSRVDescriptorHandle*)texture->GetSRVDescriptor();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srv->GetHandle().gpu;

		if (isComputeShader)
			dxCmdList.GetCommandList()->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			dxCmdList.GetCommandList()->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	void RecoardSetRWTextureCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		SetRWTextureCommand& setTextureCommandData = (SetRWTextureCommand&)commandData;

		bool isComputeShader = setTextureCommandData.m_IsComputeShader;
		uint32 index = setTextureCommandData.m_Index;
		Texture2DUAVDescriptorHandle* uav = setTextureCommandData.m_UAVHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = ((DirectX12Texture2DUAVDescriptorHandle*)uav)->GetHandle().gpu;

		if (isComputeShader)
			dxCmdList.GetCommandList()->SetComputeRootDescriptorTable(index, gpuHandle);
		else
			dxCmdList.GetCommandList()->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}


	// Execution

	void RecoardDrawMeshCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		DrawMeshCommand& drawCommandData = (DrawMeshCommand&)commandData;

		Mesh* mesh = drawCommandData.m_Mesh;
		InstanceBuffer* instanceBuffer = drawCommandData.m_Instances;
		uint32 numInstances = drawCommandData.m_NumberOfInstances;

		uint32 indexCount = mesh->GetIndexBuffer()->GetResource()->GetCount();
		DirectX12VertexBufferView* vertexBufferView = (DirectX12VertexBufferView*)mesh->GetVertexBuffer()->GetView();
		DirectX12IndexBufferView* indexBufferView = (DirectX12IndexBufferView*)mesh->GetIndexBuffer()->GetView();

		if (instanceBuffer)
		{
			DirectX12InstanceBufferView* instanceBufferView = (DirectX12InstanceBufferView*)instanceBuffer->GetView();

			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView(), instanceBufferView->GetView() };
			dxCmdList.GetCommandList()->IASetVertexBuffers(0, 2, views);
			dxCmdList.GetCommandList()->IASetIndexBuffer(&indexBufferView->GetView());
			dxCmdList.GetCommandList()->DrawIndexedInstanced(indexCount, numInstances, 0, 0, 0);
		}
		else
		{
			D3D12_VERTEX_BUFFER_VIEW views[] = { vertexBufferView->GetView() };
			dxCmdList.GetCommandList()->IASetVertexBuffers(0, 1, views);
			dxCmdList.GetCommandList()->IASetIndexBuffer(&indexBufferView->GetView());
			dxCmdList.GetCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
		}
	}

	void RecoardDisbatchComputeCommand(CommandList& cmdList, Command& commandData)
	{
		DirectX12CommandList& dxCmdList = (DirectX12CommandList&)cmdList;
		DisbatchComputeCommand& disbtachCommandData = (DisbatchComputeCommand&)commandData;

		uint32 tgx = disbtachCommandData.m_ThreadGroupsX;
		uint32 tgy = disbtachCommandData.m_ThreadGroupsY;
		uint32 tgz = disbtachCommandData.m_ThreadGroupsZ;

		dxCmdList.GetCommandList()->Dispatch(tgx, tgy, tgz);
	}


	// init
	void InitDirectX12Commands()
	{
#define SetCommandFunctionPointer(commandStruct) commandStruct::RecoardCommandFunc = Recoard##commandStruct;
		SetCommandFunctionPointer(TransitionCommand);
		SetCommandFunctionPointer(SetRenderTargetCommand);
		SetCommandFunctionPointer(ClearRenderTargetCommand);
		SetCommandFunctionPointer(SetShaderCommand);
		SetCommandFunctionPointer(SetRootConstantCommand);
		SetCommandFunctionPointer(SetConstantBufferCommand);
		SetCommandFunctionPointer(SetStructuredBufferCommand);
		SetCommandFunctionPointer(SetTextureCommand);
		SetCommandFunctionPointer(SetRWTextureCommand);
		SetCommandFunctionPointer(DrawMeshCommand);
		SetCommandFunctionPointer(DisbatchComputeCommand);
#undef  SetCommandFunctionPointer;
	}



}

