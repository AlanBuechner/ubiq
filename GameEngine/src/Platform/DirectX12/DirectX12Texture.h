#pragma once
#include "Engine/Renderer/Texture.h"
#include "DirectX12Descriptors.h"
#include "DX.h"

namespace Engine
{
	class DirectX12Texture2D : public Texture2D
	{
	public:
		DirectX12Texture2D(const fs::path& path, Ref<TextureAttribute> attrib);
		DirectX12Texture2D(const uint32, const uint32 height, Ref<TextureAttribute> attrib);
		virtual ~DirectX12Texture2D();

		virtual uint32 GetWidth() const override { return m_Width; }
		virtual uint32 GetHeight() const override { return m_Height; }
		virtual void* GetTextureHandle() const override { return (void*)m_Handle.gpu.ptr; }
		virtual void* GetSamplerHandle() const override { return (void*)m_SamplerHandle.gpu.ptr; }
		virtual uint32 GetDescriptorLocation() const override { return m_Handle.GetIndex(); }

		virtual void SetData(void* data) override;
		virtual void LoadFromFile(const fs::path& path);
		virtual Ref<TextureAttribute> GetAttributes() const override { return m_Attribute; }

		const DirectX12DescriptorHandle& GetHandle() { return m_Handle; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_Handle.cpu.ptr == ((DirectX12Texture2D&)other).m_Handle.cpu.ptr;
		}

	private:
		void CreateImage(uint32 width, uint32 height);
		void CreateSampler();

	private:
		fs::path m_Path;

		uint32 m_Width, m_Height;
		Ref<TextureAttribute> m_Attribute;

		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_Handle;
		DirectX12DescriptorHandle m_SamplerHandle;

		bool m_UseMipMaps = true;

	};
}
