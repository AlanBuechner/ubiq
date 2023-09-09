#include "pch.h"
#include "DirectX12Texture.h"
#include "Directx12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{


	DXGI_FORMAT GetDXGITextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA16:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::RGBA32:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case TextureFormat::RG16:
			return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::RG32:
			return DXGI_FORMAT_R32G32_FLOAT;

		case TextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;
		case TextureFormat::DEPTH24STENCIL8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			break;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	DXGI_FORMAT GetDXGISRVTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA16:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::RGBA32:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case TextureFormat::RG16:
			return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::RG32:
			return DXGI_FORMAT_R32G32_FLOAT;

		case TextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;
		case TextureFormat::DEPTH24STENCIL8:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:
			break;
		}
		return DXGI_FORMAT_UNKNOWN;
	}



	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips, TextureFormat format)
	{
		m_DefultState = ResourceState::ShaderResource;
		m_Width = width;
		m_Height = height;
		m_Mips = numMips == 0 ? (uint32)std::floor(std::log2(std::max(width, height))) + 1 : numMips;
		m_Format = format;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		{ // create texture buffer
			CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC rDesc;
			rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			rDesc.Format = GetDXGIFormat();
			rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			rDesc.Flags = D3D12_RESOURCE_FLAG_NONE | IsDepthStencil(GetFormat()) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			rDesc.MipLevels = m_Mips;
			rDesc.Width = width;
			rDesc.Height = height;
			rDesc.Alignment = 0;
			rDesc.DepthOrArraySize = 1;
			rDesc.SampleDesc = { 1, 0 };

			context->GetDevice()->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&rDesc,
				(D3D12_RESOURCE_STATES)GetState(m_DefultState),
				nullptr,
				IID_PPV_ARGS(&m_Buffer)
			);
		}

		{ // create upload buffer
			D3D12_RESOURCE_DESC rDesc;
			UINT stride = GetStride();
			UINT uploadPitch = (m_Width * stride + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
			UINT uploadSize = m_Height * uploadPitch;

			rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rDesc.Alignment = 0;
			rDesc.Width = uploadSize;
			rDesc.Height = 1;
			rDesc.DepthOrArraySize = 1;
			rDesc.MipLevels = 1;
			rDesc.Format = DXGI_FORMAT_UNKNOWN;
			rDesc.SampleDesc.Count = 1;
			rDesc.SampleDesc.Quality = 0;
			rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

			CORE_ASSERT_HRESULT(
				context->GetDevice()->CreateCommittedResource(
					&props,
					D3D12_HEAP_FLAG_NONE,
					&rDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(&m_UploadBuffer)
				), "Failed To Create Upload Buffer"
			);
		}
	}

	DirectX12Texture2DResource::~DirectX12Texture2DResource()
	{
		CORE_INFO("Deleting {0}", (uint64)m_Buffer);

		m_Buffer->Release();
		//m_Buffer = nullptr;

		m_UploadBuffer->Release();
		m_UploadBuffer = nullptr;

		m_Width = -1;
	}

	void DirectX12Texture2DResource::SetData(void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		UINT stride = GetStride();
		UINT uploadPitch = (m_Width * stride + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = m_Height * uploadPitch;

		// map the date to the upload buffer
		void* mapped = nullptr;
		D3D12_RANGE range = { 0, uploadSize };
		CORE_ASSERT_HRESULT(m_UploadBuffer->Map(0, &range, &mapped), "Failed to map uplaod buffer");
		for (uint32 y = 0; y < m_Height; y++)
		{
			byte* pScan = (byte*)mapped + y * uploadPitch;
			memcpy(pScan, (byte*)data + y * m_Width * 4, m_Width * 4);
		}
		m_UploadBuffer->Unmap(0, &range);

		context->GetDX12ResourceManager()->UploadTexture(m_Buffer, m_UploadBuffer, m_Width, m_Height,
			uploadPitch, m_Mips, (D3D12_RESOURCE_STATES)GetState(m_DefultState), GetDXGIFormat());
	}

	uint32 DirectX12Texture2DResource::GetState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::Common:
			return D3D12_RESOURCE_STATE_COMMON;
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | (IsDepthStencil(m_Format) ? D3D12_RESOURCE_STATE_DEPTH_READ : 0);
		case ResourceState::UnorderedResource:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ResourceState::RenderTarget:
			return IsDepthStencil(m_Format) ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;
		default:
			break;
		}
	}


	DirectX12Texture2DSRVDescriptorHandle::DirectX12Texture2DSRVDescriptorHandle()
	{
		m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12Texture2DSRVDescriptorHandle::ReBind(Texture2DResource* resource)
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
	}

	DirectX12Texture2DRTVDSVDescriptorHandle::DirectX12Texture2DRTVDSVDescriptorHandle(Texture2DResource* resource)
	{
		m_DSV = IsDepthStencil(resource->GetFormat());
		if (m_DSV)
			m_RTVDSVHandle = DirectX12ResourceManager::s_DSVHeap->Allocate();
		else
			m_RTVDSVHandle = DirectX12ResourceManager::s_RTVHeap->Allocate();
	}

	void DirectX12Texture2DRTVDSVDescriptorHandle::ReBind(Texture2DResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12Texture2DResource* dxResource = (DirectX12Texture2DResource*)resource;
		bool DSV = IsDepthStencil(resource->GetFormat());

		// no api to change the format so this should never trigger but i want it here just in case
		CORE_ASSERT(DSV == m_DSV, m_DSV ? "cannot rebind depth stencel as render target" : "cannot rebind render target as depth stencil");

		if (DSV)
			context->GetDevice()->CreateDepthStencilView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);
		else
			context->GetDevice()->CreateRenderTargetView(dxResource->GetBuffer(), nullptr, m_RTVDSVHandle.cpu);
	}

}
