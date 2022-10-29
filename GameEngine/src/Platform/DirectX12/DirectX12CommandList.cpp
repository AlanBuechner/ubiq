#include "pch.h"
#include "Engine/Math/Math.h"
#include "Engine/Renderer/Renderer.h"

#include "DirectX12CommandList.h"
#include "DirectX12Context.h"
#include "DirectX12Shader.h"
#include "DirectX12Buffer.h"
#include "DirectX12ConstantBuffer.h"
#include "DirectX12Texture.h"
#include "DirectX12InstanceBuffer.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Util/Performance.h"

namespace Engine
{
	static inline D3D12_RESOURCE_BARRIER Transition(
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
		m_Allocators.resize(numAllocators);
		D3D12_COMMAND_LIST_TYPE type = GetD3D12CommandListType(m_Type);
		for (uint32 i = 0; i < numAllocators; i++)
		{
			CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(m_Allocators[i].GetAddressOf())),
				"Faild to Create Command List Allocator");
			m_Allocators[i]->SetName(L"Command Allocator");
		}
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandList(0, type, m_Allocators[0].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())),
			"Faild to Create Commnd List");
		m_CommandList->SetName(L"Command List");
		m_CommandList->Close();

		m_RecordFlag.Signal();
	}

	void DirectX12CommandList::StartRecording(Ref<ShaderPass> startShader)
	{
		m_RecordFlag.Wait();
		m_RecordFlag.Clear();
		m_RenderTarget = nullptr;
		wrl::ComPtr<ID3D12CommandAllocator> allocator = GetAllocator();
		CORE_ASSERT_HRESULT(allocator->Reset(), "Failed to reset command allocator");
		ID3D12PipelineState* startPipeline = nullptr;
		Ref<DirectX12Shader> dxShader = std::dynamic_pointer_cast<DirectX12Shader>(startShader);
		if (startShader)
			startPipeline = dxShader->GetPipelineState().Get();
		CORE_ASSERT_HRESULT(m_CommandList->Reset(allocator.Get(), startPipeline), "Failed to reset command list");

		if (startShader)
		{
			m_CommandList->SetGraphicsRootSignature(dxShader->GetRootSignature().Get());
			m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		ID3D12DescriptorHeap* heaps[]{ DirectX12ResourceManager::s_SRVHeap->GetHeap().Get(), DirectX12ResourceManager::s_SamplerHeap->GetHeap().Get() };
		m_CommandList->SetDescriptorHeaps(2, heaps);
	}

	void DirectX12CommandList::SetRenderTarget(Ref<FrameBuffer> buffer)
	{
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not set render targets. command will be ignored");
			return;
		}
		m_RenderTarget = std::dynamic_pointer_cast<DirectX12FrameBuffer>(buffer);
		std::vector<FrameBufferTextureSpecification> attachments = m_RenderTarget->GetSpecification().Attachments.Attachments;
		std::vector<D3D12_RESOURCE_BARRIER> barriers(attachments.size());
		std::vector<uint64> rendertargetHandles(attachments.size() - (size_t)m_RenderTarget->HasDepthAttachment());
		uint64 depthHandle = 0;
		for (uint32 i = 0; i < barriers.size(); i++)
		{
			bool swapChainTarget = m_RenderTarget->GetSpecification().SwapChainTarget;
			if (attachments[i].IsDepthStencil())
			{
				barriers[i] = Transition(m_RenderTarget->GetBuffer(i).Get(), m_RenderTarget->GetDXDepthState(), DirectX12FrameBuffer::GetDXDepthState(FrameBuffer::RenderTarget));
				depthHandle = m_RenderTarget->GetAttachmentRenderHandle(i);
			}
			else
			{
				barriers[i] = Transition(m_RenderTarget->GetBuffer(i).Get(), m_RenderTarget->GetDXState(), DirectX12FrameBuffer::GetDXState(FrameBuffer::RenderTarget));
				rendertargetHandles[i] = m_RenderTarget->GetAttachmentRenderHandle(i);
			}
		}
		if (m_RenderTarget->GetState() != FrameBuffer::RenderTarget)
			m_CommandList->ResourceBarrier((uint32)barriers.size(), barriers.data());
		m_CommandList->OMSetRenderTargets((uint32)rendertargetHandles.size(), (D3D12_CPU_DESCRIPTOR_HANDLE*)rendertargetHandles.data(), FALSE, depthHandle ? (D3D12_CPU_DESCRIPTOR_HANDLE*)&depthHandle : nullptr);
		LONG width = (LONG)m_RenderTarget->GetSpecification().Width;
		LONG height = (LONG)m_RenderTarget->GetSpecification().Height;
		const D3D12_VIEWPORT viewport = { 0, 0, (FLOAT)width, (FLOAT)height, 0, 1 };
		m_CommandList->RSSetViewports(1, &viewport);
		const D3D12_RECT r = { 0, 0, width, height };
		m_CommandList->RSSetScissorRects(1, &r);

		m_RenderTarget->SetState(FrameBuffer::RenderTarget);
	}

	void DirectX12CommandList::ClearRenderTarget()
	{
		ClearRenderTarget(m_RenderTarget);
	}

	void DirectX12CommandList::ClearRenderTarget(uint32 attachment)
	{
		ClearRenderTarget(m_RenderTarget, attachment);
	}

	void DirectX12CommandList::ClearRenderTarget(uint32 attachment, const Math::Vector4& color)
	{
		ClearRenderTarget(m_RenderTarget, attachment, color);
	}

	void DirectX12CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer)
	{
		for (uint32 i = 0; i < frameBuffer->GetSpecification().Attachments.Attachments.size(); i++)
			ClearRenderTarget(frameBuffer, i);
	}

	void DirectX12CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment)
	{
		Math::Vector4 color = frameBuffer->GetSpecification().Attachments.Attachments[attachment].ClearColor;
		ClearRenderTarget(frameBuffer, attachment, color);
	}

	void DirectX12CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color)
	{
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not clear render tartets. command will be ignored");
			return;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = frameBuffer->GetAttachmentRenderHandle(attachment);
		if (frameBuffer->GetSpecification().Attachments.Attachments[attachment].IsDepthStencil())
			m_CommandList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, color.r, (uint8)color.g, 0, nullptr);
		else
			m_CommandList->ClearRenderTargetView(handle, (float*)&color, 0, nullptr);

		std::dynamic_pointer_cast<DirectX12FrameBuffer>(frameBuffer)->Clear();
	}



	void DirectX12CommandList::SetShader(Ref<ShaderPass> shader)
	{
		Ref<DirectX12Shader> dxShader = std::dynamic_pointer_cast<DirectX12Shader>(shader);
		m_CommandList->SetPipelineState(dxShader->GetPipelineState().Get());
		m_CommandList->SetGraphicsRootSignature(dxShader->GetRootSignature().Get());

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

		m_CommandList->IASetPrimitiveTopology(top);

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
					m_CommandList->SetGraphicsRootDescriptorTable(p.rootIndex, DirectX12ResourceManager::s_SRVHeap->GetGPUHeapStart());
					break;
				default:
					break;
				}
			}
		}

	}

	void DirectX12CommandList::SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer)
	{
		Ref<DirectX12ConstantBuffer> cb = std::dynamic_pointer_cast<DirectX12ConstantBuffer>(buffer);
		m_CommandList->SetGraphicsRootConstantBufferView(index, cb->GetBuffer()->GetGPUVirtualAddress());
	}

	void DirectX12CommandList::SetTexture(uint32 index, Ref<Texture> texture)
	{
		Ref<DirectX12Texture2D> d3dTexture = std::dynamic_pointer_cast<DirectX12Texture2D>(texture);
		m_CommandList->SetGraphicsRootDescriptorTable(index, d3dTexture->GetHandle().gpu);
	}

	void DirectX12CommandList::DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer, int numInstances)
	{
		Ref<DirectX12VertexBuffer> vb = std::dynamic_pointer_cast<DirectX12VertexBuffer>(mesh->GetVertexBuffer());
		Ref<DirectX12IndexBuffer> ib = std::dynamic_pointer_cast<DirectX12IndexBuffer>(mesh->GetIndexBuffer());
		Ref<DirectX12InstanceBuffer> isb = std::dynamic_pointer_cast<DirectX12InstanceBuffer>(instanceBuffer);

		if (numInstances < 0)
			numInstances = isb->GetCount();

		D3D12_VERTEX_BUFFER_VIEW views[] = { vb->GetView(), isb->GetView() };
		m_CommandList->IASetVertexBuffers(0, 2, views);
		m_CommandList->IASetIndexBuffer(&ib->GetView());
		m_CommandList->DrawIndexedInstanced(ib->GetCount(), numInstances, 0, 0, 0);
	}

	void DirectX12CommandList::Present(Ref<FrameBuffer> fb)
	{
		if (fb == nullptr)
			fb = m_RenderTarget;

		if (fb && fb->GetState() != FrameBuffer::State::Common)
		{
			Ref<DirectX12FrameBuffer> dxfb = std::dynamic_pointer_cast<DirectX12FrameBuffer>(fb);
			bool swapChainTarget = dxfb->GetSpecification().SwapChainTarget;
			std::vector<FrameBufferTextureSpecification> attachments = dxfb->GetSpecification().Attachments.Attachments;
			std::vector<D3D12_RESOURCE_BARRIER> barriers(attachments.size());
			for (uint32 i = 0; i < barriers.size(); i++)
			{
				if (attachments[i].IsDepthStencil())
					barriers[i] = Transition(dxfb->GetBuffer(i).Get(), dxfb->GetDXDepthState(), DirectX12FrameBuffer::GetDXDepthState(FrameBuffer::Common));
				else
					barriers[i] = Transition(dxfb->GetBuffer(i).Get(), dxfb->GetDXState(), DirectX12FrameBuffer::GetDXState(FrameBuffer::Common));
			}
			m_CommandList->ResourceBarrier((uint32)barriers.size(), barriers.data());
			dxfb->SetState(FrameBuffer::Common);
		}
		if (fb == m_RenderTarget)
			m_RenderTarget = nullptr;
	}

	void DirectX12CommandList::ExecuteBundle(Ref<CommandList> commandList)
	{
		Ref<DirectX12CommandList> dxCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(commandList);
		if (dxCommandList->m_Type != CommandList::Bundle)
			return;

		m_CommandList->ExecuteBundle(dxCommandList->GetCommandList().Get());
	}

	void DirectX12CommandList::Close()
	{
		m_CommandList->Close();
		m_RenderTarget = nullptr;
		m_CurrentCommandAllocator = (m_CurrentCommandAllocator + 1) % m_Allocators.size();
	}

	wrl::ComPtr<ID3D12CommandAllocator> DirectX12CommandList::GetAllocator()
	{
		return m_Allocators[m_CurrentCommandAllocator];
	}

}
