#include "pch.h"
#include "DirectX12FrameBuffer.h"
#include "Directx12Context.h"
#include "DirectX12ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Util/Performance.h"

namespace Engine
{

	DXGI_FORMAT UbiqToDXGI(FrameBufferTextureFormat format)
	{
		switch (format)
		{

		case Engine::FrameBufferTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Engine::FrameBufferTextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;
		case Engine::FrameBufferTextureFormat::DEPTH24STENCIL8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			break;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	DirectX12FrameBuffer::DirectX12FrameBuffer(const FrameBufferSpecification& spec) :
		m_Spec(spec)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		for (auto format : m_Spec.Attachments.Attachments)
		{
			if (format.IsDepthStencil())
			{
				CORE_ASSERT(!HasDepthAttachment(), "can not have more than one depth attachment on a frame buffer");
				m_DepthAttachmentSpec = format;
			}
			m_AttachmentSpecs.emplace_back(format);
		}

		m_TargetHandles.resize(m_AttachmentSpecs.size());
		m_SRVHandles.resize(m_AttachmentSpecs.size());
		m_Buffers.resize(m_AttachmentSpecs.size());
		if (!spec.SwapChainTarget)
		{
			for (uint32 i = 0; i < m_AttachmentSpecs.size(); i++)
			{
				if (m_AttachmentSpecs[i].IsDepthStencil())
					m_TargetHandles[i] = DirectX12ResourceManager::s_DSVHeap->Allocate();
				else
				{
					m_TargetHandles[i] = DirectX12ResourceManager::s_RTVHeap->Allocate();
					m_SRVHandles[i] = DirectX12ResourceManager::s_SRVHeap->Allocate();
				}
			}
		}
		Invalidate();
	}

	DirectX12FrameBuffer::~DirectX12FrameBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		for (auto& dh : m_TargetHandles)
			dh.Free();
		for (auto& dh : m_SRVHandles)
			dh.Free();

		for(auto& buffer : m_Buffers)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(buffer);
	}

	void DirectX12FrameBuffer::Invalidate()
	{
		CREATE_PROFILE_FUNCTIONI();
		if (!m_Spec.SwapChainTarget)
		{
			for (uint32 i = 0; i < m_AttachmentSpecs.size(); i++)
				CreateAttachment(i);
		}
	}

	void DirectX12FrameBuffer::Resize(uint32 width, uint32 height)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_Spec.Width = width;
		m_Spec.Height = height;

		Invalidate();
	}

	uint64 DirectX12FrameBuffer::GetAttachmentRenderHandle(uint32 index) const
	{
		CORE_ASSERT(index < m_TargetHandles.size(), "");
		return m_TargetHandles[index].cpu.ptr;
	}

	uint64 DirectX12FrameBuffer::GetAttachmentShaderHandle(uint32 index) const
	{
		CORE_ASSERT(index < m_SRVHandles.size(), "");
		return m_SRVHandles[index].gpu.ptr;
	}

	int DirectX12FrameBuffer::ReadPixle(uint32 index, int x, int y)
	{
		// TODO
		return 0;
		int val;
		m_Buffers[index]->ReadFromSubresource(&val, x, y, 0, nullptr);
		return val;
	}

	void DirectX12FrameBuffer::CreateAttachment(uint32 i)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_HEAP_PROPERTIES props
		{
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			0, 0
		};

		DXGI_FORMAT format = UbiqToDXGI(m_Spec.Attachments.Attachments[i].TextureFormat);

		D3D12_RESOURCE_DESC rDesc;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Format = format;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = m_Spec.Attachments.Attachments[i].IsDepthStencil() ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		rDesc.MipLevels = 1;
		rDesc.Width = m_Spec.Width;
		rDesc.Height = m_Spec.Height;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 1;
		rDesc.SampleDesc = { 1, 0 };

		Math::Vector4 color = m_Spec.Attachments.Attachments[i].ClearColor;
		D3D12_CLEAR_VALUE clearVal{};
		clearVal.Format = format;
		((Math::Vector4&)clearVal.Color) = color;

		D3D12_RESOURCE_STATES state = m_Spec.Attachments.Attachments[i].IsDepthStencil() ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_COMMON;
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommittedResource(
			&props, D3D12_HEAP_FLAG_NONE, &rDesc,
			state, &clearVal, IID_PPV_ARGS(m_Buffers[i].GetAddressOf())
		),"Failed To Create Resorce");

		if (!m_Spec.Attachments.Attachments[i].IsDepthStencil())
		{
			context->GetDevice()->CreateRenderTargetView(m_Buffers[i].Get(), nullptr, m_TargetHandles[i].cpu);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = rDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			context->GetDevice()->CreateShaderResourceView(m_Buffers[i].Get(), &srvDesc, m_SRVHandles[i].cpu);
		}
		else
		{
			context->GetDevice()->CreateDepthStencilView(m_Buffers[i].Get(), nullptr, m_TargetHandles[i].cpu);
		}
	}

}
