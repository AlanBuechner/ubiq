#include "pch.h"
#include "DirectX12Texture.h"
#include "Directx12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"
#include "stb_image.h"

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



	DirectX12Texture2DResource::DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips)
	{
		m_NumMips = numMips == 0 ? (uint32)std::floor(std::log2(std::max(width, height))) + 1 : numMips;
		Resize(width, height);
	}

	DirectX12Texture2DResource::~DirectX12Texture2DResource()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_SRVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_SRVHandle);
		for (auto& handle : m_UAVHandles) context->GetDX12ResourceManager()->ScheduleHandleDeletion(handle);

		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12Texture2DResource::Resize(uint32 width, uint32 height)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_Buffer)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);

		m_Width = width;
		m_Height = height;

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC rDesc;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Format = GetDXGIFormat();
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.MipLevels = m_NumMips;
		rDesc.Width = width;
		rDesc.Height = height;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 1;
		rDesc.SampleDesc = { 1, 0 };

		context->GetDevice()->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&rDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(m_Buffer.GetAddressOf())
		);
	}

	void DirectX12Texture2DResource::SetData(void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		// create upload buffer
		D3D12_RESOURCE_DESC rDesc;
		UINT uploadPitch = (m_Width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
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

		wrl::ComPtr<ID3D12Resource> uploadBuffer;
		CORE_ASSERT_HRESULT(
			context->GetDevice()->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&rDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(uploadBuffer.GetAddressOf())
			), "Failed To Create Upload Buffer"
		);

		// map the date to the upload buffer
		void* mapped = nullptr;
		D3D12_RANGE range = { 0, uploadSize };
		CORE_ASSERT_HRESULT(uploadBuffer->Map(0, &range, &mapped), "Failed to map uplaod buffer");
		for (uint32 y = 0; y < m_Height; y++)
		{
			byte* pScan = (byte*)mapped + y * uploadPitch;
			memcpy(pScan, (byte*)data + y * m_Width * 4, m_Width * 4);
		}
		uploadBuffer->Unmap(0, &range);

		context->GetDX12ResourceManager()->UploadTexture(m_Buffer, uploadBuffer, m_Width, m_Height, uploadPitch, true, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(uploadBuffer);
	}

	void DirectX12Texture2DResource::CreateSRVHandle()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (!m_SRVHandle)
		{
			m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = GetDXGIFormat();
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_NumMips;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			context->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_SRVHandle.cpu);
		}
	}

	void DirectX12Texture2DResource::CreateUAVHandle()
	{

	}







	DirectX12Texture2D::DirectX12Texture2D(const fs::path& path)
	{
		LoadFromFile(path);
		m_Resource->CreateSRVHandle();
	}

	DirectX12Texture2D::DirectX12Texture2D(const uint32 width, const uint32 height)
	{
		m_Resource = CreateRef<DirectX12Texture2DResource>(width, height, 0);
		m_Resource->CreateSRVHandle();
	}

	DirectX12Texture2D::DirectX12Texture2D(Ref<Texture2DResource> resource)
	{
		m_Resource = std::dynamic_pointer_cast<DirectX12Texture2DResource>(resource);
		m_Resource->CreateSRVHandle();
	}

	void DirectX12Texture2D::LoadFromFile(const fs::path& path)
	{
		InstrumentationTimer timer = CREATE_PROFILEI();
		m_Path = path;

		timer.Start("LoadImage");
		int width, height, channels;
		stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
		CORE_ASSERT(data, "Failed to load image \"{0}\"", path.string());
		timer.End();

		if (m_Resource)
			m_Resource->Resize(width, height);
		else
			m_Resource = CreateRef<DirectX12Texture2DResource>(width, height, 0);
		m_Resource->SetData(data);

		stbi_image_free(data);
	}
}
