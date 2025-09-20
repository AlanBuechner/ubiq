#include "pch.h"
#include "DirectX12Texture.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{

	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, TextureFormat format, ID3D12Resource* resource)
	{
		m_Buffer = resource;
		m_Width = width;
		m_Height = height;
		m_Mips = 1;
		m_Format = format;
		m_SampleCount = MSAASampleCount::MSAA1;
	}


	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips, TextureFormat format, Math::Vector4 clearColor, TextureType type, MSAASampleCount sampleCount)
	{
		m_DefultState = ResourceState::ShaderResource;
		m_Width = width;
		m_Height = height;
		m_Mips = FixMipLevels(numMips, width, height);
		m_Format = format;
		m_ClearColor = clearColor;
		m_Type = type;
		m_SampleCount = sampleCount;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();


		m_TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		m_TextureDesc.Format = GetDXGIFormat();
		m_TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		m_TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if ((uint32)m_Type & (uint32)TextureType::RenderTarget)
			m_TextureDesc.Flags |= (IsTextureFormatDepthStencil(GetFormat()) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		if ((uint32)m_Type & (uint32)TextureType::RWTexture)
			m_TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_TextureDesc.MipLevels = m_Mips;
		m_TextureDesc.Width = width;
		m_TextureDesc.Height = height;
		m_TextureDesc.Alignment = 0;
		m_TextureDesc.DepthOrArraySize = 1;
		m_TextureDesc.SampleDesc = { (uint32)sampleCount, 0 };

		if (m_Transient)
		{
			// attempt small resource placement first
			m_TextureDesc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			D3D12_RESOURCE_ALLOCATION_INFO info = context->GetDevice()->GetResourceAllocationInfo(0, 1, &m_TextureDesc);

			if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			{
				// If the alignment requested is not granted, then let D3D tell us
				// the alignment that needs to be used for these resources.
				m_TextureDesc.Alignment = 0;
				info = context->GetDevice()->GetResourceAllocationInfo(0, 1, &m_TextureDesc);
			}

			m_AllocationInfo.size = info.SizeInBytes;
			m_AllocationInfo.allignment = info.Alignment;
		}
		else
		{
			// create texture buffer
			CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_CLEAR_VALUE clearValue = {};
			for(uint32 i = 0; i < 4; i++)
				clearValue.Color[i] = ((float*)&m_ClearColor)[i];

			context->GetDevice()->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&m_TextureDesc,
				(D3D12_RESOURCE_STATES)GetGPUState(m_DefultState),
				nullptr,
				IID_PPV_ARGS(&m_Buffer)
			);
		}
	}

	DirectX12Texture2DResource::~DirectX12Texture2DResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void DirectX12Texture2DResource::SetData(void* data)
	{
		// create upload buffer
		UploadTextureResource* uploadBuffer = UploadTextureResource::Create(m_Width, m_Height, m_Mips, m_Format);

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		UINT stride = GetStride();
		UINT uploadPitch = (m_Width * stride + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = m_Height * uploadPitch;

		// map the date to the upload buffer
		void* mapped = uploadBuffer->Map();
		for (uint32 y = 0; y < m_Height; y++)
		{
			byte* pScan = (byte*)mapped + y * uploadPitch;
			memcpy(pScan, (byte*)data + y * m_Width * 4, m_Width * 4);
		}
		uploadBuffer->UnMap();

		// submit upload
		context->GetDX12ResourceManager()->UploadTexture(this, uploadBuffer, m_Width, m_Height, m_Mips, m_DefultState, m_Format);

		// schedule the upload buffer to be deleted
		context->GetResourceManager()->ScheduleResourceDeletion(uploadBuffer);
	}

	uint32 DirectX12Texture2DResource::GetGPUState(ResourceState state) const
	{
		switch (state)
		{
		case ResourceState::Common:
			return D3D12_RESOURCE_STATE_COMMON;
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | (IsTextureFormatDepthStencil(m_Format) ? D3D12_RESOURCE_STATE_DEPTH_READ : 0);
		case ResourceState::UnorderedResource:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ResourceState::RenderTarget:
			return IsTextureFormatDepthStencil(m_Format) ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		default: 
			return D3D12_RESOURCE_STATE_COMMON;
		}
	}


	void DirectX12Texture2DResource::AllocateTransient(TransientResourceHeap* heap, uint32 offset)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		DirectX12TransientResourceHeap* dxHeap = (DirectX12TransientResourceHeap*)heap;

		context->GetDevice()->CreatePlacedResource(
			dxHeap->GetHeap(),
			m_AllocationInfo.size,
			&m_TextureDesc,
			(D3D12_RESOURCE_STATES)GetGPUState(m_DefultState),
			nullptr,
			IID_PPV_ARGS(&m_Buffer)
		);
	}

	DirectX12Texture2DSRVDescriptorHandle::DirectX12Texture2DSRVDescriptorHandle(Texture2DResource* resource) :
		Texture2DSRVDescriptorHandle(resource)
	{
		m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12Texture2DSRVDescriptorHandle::Bind()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)m_Resource;

		// check if resource has been allocated
		if (dxResource->GetBuffer() == nullptr)
			return;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dxResource->GetDXGISRVFormat();
		srvDesc.ViewDimension = m_Resource->UsingMSAA() ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = dxResource->GetMips();
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		context->GetDevice()->CreateShaderResourceView(dxResource->GetBuffer(), &srvDesc, m_SRVHandle.cpu);

	}



	DirectX12Texture2DUAVDescriptorHandle::DirectX12Texture2DUAVDescriptorHandle(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height) :
		Texture2DUAVDescriptorHandle(resource, mipSlice, width, height)
	{
		m_UAVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12Texture2DUAVDescriptorHandle::Bind()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)m_Resource;

		// check if resource has been allocated
		if (dxResource->GetBuffer() == nullptr)
			return;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = dxResource->GetDXGISRVFormat();
		uavDesc.ViewDimension = m_Resource->UsingMSAA() ? D3D12_UAV_DIMENSION_TEXTURE2DMS : D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = m_MipSlice;
		context->GetDevice()->CreateUnorderedAccessView(dxResource->GetBuffer(), nullptr, &uavDesc, m_UAVHandle.cpu);
	}




	DirectX12Texture2DRTVDSVDescriptorHandle::DirectX12Texture2DRTVDSVDescriptorHandle(Texture2DResource* resource) :
		Texture2DRTVDSVDescriptorHandle(resource)
	{
		m_DSV = IsTextureFormatDepthStencil(resource->GetFormat());
		if (m_DSV)
			m_RTVDSVHandle = DirectX12ResourceManager::s_DSVHeap->Allocate();
		else
			m_RTVDSVHandle = DirectX12ResourceManager::s_RTVHeap->Allocate();
	}

	void DirectX12Texture2DRTVDSVDescriptorHandle::Bind()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)m_Resource;

		// check if resource has been allocated
		if (dxResource->GetBuffer() == nullptr)
			return;

		bool DSV = IsTextureFormatDepthStencil(m_Resource->GetFormat());

		// no api to change the format so this should never trigger but i want it here just in case
		CORE_ASSERT(DSV == m_DSV, m_DSV ? "cannot rebind depth stencel as render target" : "cannot rebind render target as depth stencil");

		if (DSV)
			context->GetDevice()->CreateDepthStencilView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);
		else
			context->GetDevice()->CreateRenderTargetView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);
	}

}
