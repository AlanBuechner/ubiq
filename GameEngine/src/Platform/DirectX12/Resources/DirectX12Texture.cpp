#include "pch.h"
#include "DirectX12Texture.h"
#include "Platform/DirectX12/Directx12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{

	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, TextureFormat format, ID3D12Resource* resource)
	{
		m_Buffer = resource;
		m_UploadBuffer = nullptr;
		m_Width = width;
		m_Height = height;
		m_Mips = 1;
		m_Format = format;
	}


	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips, TextureFormat format, Math::Vector4 clearColor, TextureType type)
	{
		m_DefultState = ResourceState::ShaderResource;
		m_Width = width;
		m_Height = height;
		m_Mips = FixMipLevels(numMips, width, height);
		m_Format = format;
		m_ClearColor = clearColor;
		m_Type = type;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		{ // create texture buffer
			CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC rDesc;
			rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			rDesc.Format = GetDXGIFormat();
			rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			if((uint32)m_Type & (uint32)TextureType::RenderTarget)
				rDesc.Flags |= (IsTextureFormatDepthStencil(GetFormat()) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
			if ((uint32)m_Type & (uint32)TextureType::RWTexture)
				rDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			rDesc.MipLevels = m_Mips;
			rDesc.Width = width;
			rDesc.Height = height;
			rDesc.Alignment = 0;
			rDesc.DepthOrArraySize = 1;
			rDesc.SampleDesc = { 1, 0 };

			D3D12_CLEAR_VALUE clearValue = {};
			for(uint32 i = 0; i < 4; i++)
				clearValue.Color[i] = ((float*)&m_ClearColor)[i];

			context->GetDevice()->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&rDesc,
				(D3D12_RESOURCE_STATES)GetState(m_DefultState),
				nullptr,
				IID_PPV_ARGS(&m_Buffer)
			);
		}

		// create upload buffer
		m_UploadBuffer = UploadTextureResource::Create(width, height, numMips, format);
	}

	DirectX12Texture2DResource::~DirectX12Texture2DResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
		delete m_UploadBuffer;
		m_UploadBuffer = nullptr;
	}

	void DirectX12Texture2DResource::SetData(void* data)
	{
		if (!m_UploadBuffer)
		{
			CORE_WARN("missing upload resource");
			return;
		}

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		UINT stride = GetStride();
		UINT uploadPitch = (m_Width * stride + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = m_Height * uploadPitch;

		// map the date to the upload buffer
		void* mapped = m_UploadBuffer->Map();
		for (uint32 y = 0; y < m_Height; y++)
		{
			byte* pScan = (byte*)mapped + y * uploadPitch;
			memcpy(pScan, (byte*)data + y * m_Width * 4, m_Width * 4);
		}
		m_UploadBuffer->UnMap();

		context->GetDX12ResourceManager()->UploadTexture(this, m_UploadBuffer, m_Width, m_Height, m_Mips, m_DefultState, m_Format);
	}

	uint32 DirectX12Texture2DResource::GetState(ResourceState state)
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
			break;
		}
	}


	DirectX12Texture2DSRVDescriptorHandle::DirectX12Texture2DSRVDescriptorHandle()
	{
		m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12Texture2DSRVDescriptorHandle::Bind(Texture2DResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)resource;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dxResource->GetDXGISRVFormat();
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = dxResource->GetMips();
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		context->GetDevice()->CreateShaderResourceView(dxResource->GetBuffer(), &srvDesc, m_SRVHandle.cpu);

		m_Resource = resource;
	}



	DirectX12Texture2DUAVDescriptorHandle::DirectX12Texture2DUAVDescriptorHandle()
	{
		m_UAVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12Texture2DUAVDescriptorHandle::Bind(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height)
	{
		m_MipSlice = mipSlice;
		m_Width = width;
		m_Height = height;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)resource;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = dxResource->GetDXGISRVFormat();
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = mipSlice;
		context->GetDevice()->CreateUnorderedAccessView(dxResource->GetBuffer(), nullptr, &uavDesc, m_UAVHandle.cpu);

		m_Resource = resource;
	}




	DirectX12Texture2DRTVDSVDescriptorHandle::DirectX12Texture2DRTVDSVDescriptorHandle(Texture2DResource* resource)
	{
		m_DSV = IsTextureFormatDepthStencil(resource->GetFormat());
		if (m_DSV)
			m_RTVDSVHandle = DirectX12ResourceManager::s_DSVHeap->Allocate();
		else
			m_RTVDSVHandle = DirectX12ResourceManager::s_RTVHeap->Allocate();
	}

	void DirectX12Texture2DRTVDSVDescriptorHandle::Bind(Texture2DResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)resource;
		bool DSV = IsTextureFormatDepthStencil(resource->GetFormat());

		// no api to change the format so this should never trigger but i want it here just in case
		CORE_ASSERT(DSV == m_DSV, m_DSV ? "cannot rebind depth stencel as render target" : "cannot rebind render target as depth stencil");

		if (DSV)
			context->GetDevice()->CreateDepthStencilView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);
		else
			context->GetDevice()->CreateRenderTargetView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);

		m_Resource = resource;
	}

}
