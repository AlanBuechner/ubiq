#pragma once
#include "Engine/Renderer/Resources/Texture.h"
#include "DirectX12Descriptors.h"
#include "DX.h"

namespace Engine
{
	DXGI_FORMAT GetDXGITextureFormat(TextureFormat format);

	class DirectX12Texture2DResource : public Texture2DResource
	{
	public:
		DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips, TextureFormat format);
		~DirectX12Texture2DResource();

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }
		DirectX12DescriptorHandle& GetSRVHandle() { return m_SRVHandle; }
		DirectX12DescriptorHandle& GetRTVDSVHandle() { return m_RTVDSVHandle; }
		DirectX12DescriptorHandle& GetUAVHandle(uint32 i) { return m_UAVHandles[i]; }

		DXGI_FORMAT GetDXGIFormat() { return GetDXGITextureFormat(m_Format); }

	private:

		void Resize(uint32 width, uint32 height);
		void SetData(void* data);
		
		void CreateSRVHandle();
		void CreateRTVDSVHandle();
		void CreateUAVHandle();

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_SRVHandle;
		DirectX12DescriptorHandle m_RTVDSVHandle;
		std::vector<DirectX12DescriptorHandle> m_UAVHandles;

		uint32 m_NumMips = 0;

		friend class DirectX12Texture2D;
		friend class DirectX12RenderTarget2D;
	};


	class DirectX12Texture2D : public Texture2D
	{
	public:
		DirectX12Texture2D(const fs::path& path);
		DirectX12Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		DirectX12Texture2D(Ref<Texture2DResource> resource);

		virtual uint32 GetWidth() const override { return m_Resource->m_Width; }
		virtual uint32 GetHeight() const override { return m_Resource->m_Height; }
		virtual uint64 GetSRVHandle() const override { return m_Resource->m_SRVHandle.gpu.ptr; }
		virtual uint32 GetDescriptorLocation() const override { return m_Resource->m_SRVHandle.GetIndex(); }

		virtual Ref<Texture2DResource> GetResource() override { return m_Resource; }

		Ref<DirectX12Texture2DResource> GetDXResource() { return m_Resource; }

		virtual void SetData(void* data) override { m_Resource->SetData(data); }
		virtual void LoadFromFile(const fs::path& path) override;

		virtual bool operator==(const Texture2D& other) const override
		{
			return m_Resource->m_SRVHandle.cpu.ptr == ((DirectX12Texture2D&)other).m_Resource->m_SRVHandle.cpu.ptr;
		}

	private:
		fs::path m_Path;

		Ref<DirectX12Texture2DResource> m_Resource;

	};


	class DirectX12RenderTarget2D : public RenderTarget2D
	{
	public:
		DirectX12RenderTarget2D(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		DirectX12RenderTarget2D(Ref<Texture2DResource> resource);

		void Resize(uint32 width, uint32 height) override { m_Resource->Resize(width, height); }
		Ref<Texture2DResource> GetResource() override { return m_Resource; }
		Ref<DirectX12Texture2DResource> GetDXResource() { return m_Resource; }


	private:
		Ref<DirectX12Texture2DResource> m_Resource;


	};

}
