#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Math/Math.h"
#include "Engine/AssetManager/AssetManager.h"
#include "ResourceState.h"
#include "Descriptor.h"

namespace Engine
{
	// Texture Resource ---------------------------------------------------------- //

	enum class TextureFormat
	{
		None = 0,

		// Color
		// 4 component float
		RGBA8,
		RGBA16,
		RGBA32,

		// 2 component float
		RG16,
		RG32,

		// 1 component int
		RED_INTEGER,

		// Depth/Stencil (only used for frame buffers)
		DEPTH24STENCIL8,

		Depth = DEPTH24STENCIL8
	};
	bool IsDepthStencil(TextureFormat format);

	enum class TextureType
	{
		Texture = 0,
		RWTexture = BIT(0),
		RenderTarget = BIT(1)
	};

	class Texture2DResource : public GPUResource
	{
	public:
		Texture2DResource() = default;
		DISABLE_COPY(Texture2DResource);
		virtual ~Texture2DResource() = 0;

		uint32 GetWidth() { return m_Width; }
		uint32 GetHeight() { return m_Height; }
		uint32 GetMips() { return m_Mips; }
		TextureFormat GetFormat() { return m_Format; }
		Math::Vector4 GetClearColor() { return m_ClearColor; }
		TextureType GetTextureType() { return m_Type; }

		uint32 GetStride();

		virtual void SetData(void* data) = 0;

		static Texture2DResource* Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type);

	protected:
		virtual bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Width = 1;
		uint32 m_Height = 1;
		uint32 m_Mips = 1;
		TextureFormat m_Format = TextureFormat::RGBA8;
		Math::Vector4 m_ClearColor = { 0,0,0,0 };
		TextureType m_Type;

	};

	// Descriptor Handles ---------------------------------------------------------- //

	class Texture2DSRVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void Bind(Texture2DResource* resource) = 0;

		static Texture2DSRVDescriptorHandle* Create(Texture2DResource* resource);

		Texture2DResource* m_Resource;
	};

	class Texture2DUAVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void Bind(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height) = 0;

		static Texture2DUAVDescriptorHandle* Create(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height);

		Texture2DResource* m_Resource;
		uint32 m_MipSlice;
		uint32 m_Width, m_Height;
	};

	class Texture2DRTVDSVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void Bind(Texture2DResource* resource) = 0;

		static Texture2DRTVDSVDescriptorHandle* Create(Texture2DResource* resource);

		Texture2DResource* m_Resource;
	};


	// Texture Objects ---------------------------------------------------------- //

	class Texture2D : public Asset
	{
	protected:
		Texture2D(Texture2DResource* resource, Texture2DSRVDescriptorHandle* srv);
		Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type);
	public:
		Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		DISABLE_COPY(Texture2D);
		virtual ~Texture2D();

		uint32 GetWidth() const { return m_Resource->GetWidth(); }
		uint32 GetHeight() const { return m_Resource->GetHeight(); }
		Texture2DResource* GetResource() const { return m_Resource; }
		GPUResourceHandle GetResourceHandle()const { return (GPUResource**) & m_Resource; }
		Texture2DSRVDescriptorHandle* GetSRVDescriptor() const { return m_SRVDescriptor; }

		void SetResizable(bool resizeable) { m_Resizeable = resizeable; }
		virtual void Resize(uint32 width, uint32 height);

		void SetData(void* data) { m_Resource->SetData(data); }

		bool operator==(const Texture2D& other) const { return m_SRVDescriptor->GetGPUHandlePointer() == other.m_SRVDescriptor->GetGPUHandlePointer(); }

		static Ref<Texture2D> Create(uint32 width, uint32 height);
		static Ref<Texture2D> Create(uint32 width, uint32 height, uint32 mips);
		static Ref<Texture2D> Create(uint32 width, uint32 height, TextureFormat format);
		static Ref<Texture2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		static Ref<Texture2D> Create(const fs::path& path = "");

		static bool ValidExtension(const fs::path& ext);

	protected:
		Texture2DResource* m_Resource;
		Texture2DSRVDescriptorHandle* m_SRVDescriptor;
		bool m_Resizeable = true;
		bool m_DataOwner = true;
	};


	class RWTexture2D : public Texture2D
	{
	public:
		RWTexture2D(Texture2DResource* resource, Texture2DSRVDescriptorHandle* srv, class RenderTarget2D* owner);
		RWTexture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor);
		DISABLE_COPY(RWTexture2D);
		virtual ~RWTexture2D() override;

		Math::Vector4 GetClearColor() { return m_Resource->GetClearColor(); }

		virtual void Resize(uint32 width, uint32 height) override;

		static Ref<RWTexture2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		static Ref<RWTexture2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearClolor);

	protected:
		void GenerateUAVDescriptors();

	protected:
		std::vector<Texture2DUAVDescriptorHandle*> m_UAVDescriptors;
		class RenderTarget2D* m_Owner;

		friend class RenderTarget2D;
	};


	class RenderTarget2D : public Texture2D
	{
	public:
		RenderTarget2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, bool RWCapable);
		DISABLE_COPY(RenderTarget2D);
		virtual ~RenderTarget2D() override;

		Math::Vector4 GetClearColor() { return m_Resource->GetClearColor(); }
		Ref<RWTexture2D> GetRWTexture2D() { return m_RWTexture; }

		Texture2DRTVDSVDescriptorHandle* GetRTVDSVDescriptor() { return m_RTVDSVDescriptor; }

		virtual void Resize(uint32 width, uint32 height) override;

		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, TextureFormat format);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, TextureFormat format, bool RWCapable);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor, bool RWCapable);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, bool RWCapable);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearClolor);
		static Ref<RenderTarget2D> Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, bool RWCapable);

	protected:
		Texture2DRTVDSVDescriptorHandle* m_RTVDSVDescriptor;
		Ref<RWTexture2D> m_RWTexture = nullptr;

		friend class DirectX12SwapChain;

	};

	// Utils ---------------------------------------------------------- //

	struct TextureFile
	{
		~TextureFile();

		uint32 width;
		uint32 height;
		uint8 channels;
		bool HDR;
		void* data; // cast to uint8* if regular or to uint16* if hdr

		TextureFormat GetTextureFormat();

		void ConvertToChannels(uint8 numChannels);

		static TextureFile* LoadFile(const fs::path& file);
	};

	uint32 FixMipLevels(uint32 mips, uint32 width, uint32 height);
}
