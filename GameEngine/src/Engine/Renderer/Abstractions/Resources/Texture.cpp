#include "pch.h"
#include "Texture.h"
#include "stb_image.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/PlatformUtils/PlatformUtils.h"
#include "EngineResource.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12Texture.h"
#endif

namespace Engine
{

	// Texture Resource ---------------------------------------------------------- //

	Texture2DResource::~Texture2DResource() {}


	Texture2DResource* Texture2DResource::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type, MSAASampleCount sampleCount)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12Texture2DResource(width, height, mips, format, clearColor, type, sampleCount);
		default: return nullptr;
		}
		
	}

	bool Texture2DResource::SupportState(ResourceState state) const
	{
		switch (state)
		{
		case ResourceState::Common:
		case ResourceState::ShaderResource:
		case ResourceState::RenderTarget:
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		case ResourceState::UnorderedResource:
			return (uint32)m_Type & (uint32)TextureType::RWTexture;
		default: return false;
		}
	}

	// Descriptor Handles ---------------------------------------------------------- //

	Texture2DSRVDescriptorHandle* Texture2DSRVDescriptorHandle::Create(Texture2DResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			Texture2DSRVDescriptorHandle* handle = new DirectX12Texture2DSRVDescriptorHandle(resource);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
	}


	Texture2DUAVDescriptorHandle* Texture2DUAVDescriptorHandle::Create(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			Texture2DUAVDescriptorHandle* handle = new DirectX12Texture2DUAVDescriptorHandle(resource, mipSlice, width, height);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
	}



	Texture2DRTVDSVDescriptorHandle* Texture2DRTVDSVDescriptorHandle::Create(Texture2DResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			Texture2DRTVDSVDescriptorHandle* handle = new DirectX12Texture2DRTVDSVDescriptorHandle(resource);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
	}



	// Texture Objects ---------------------------------------------------------- //

	Texture2D::Texture2D(Texture2DResource* resource, Texture2DSRVDescriptorHandle* srv)
	{
		m_DataOwner = false;
		m_Resource = resource;
		m_SRVDescriptor = srv;
	}

	Texture2D::Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type, MSAASampleCount sampleCount)
	{
		m_Resource = Texture2DResource::Create(width, height, mips, format, clearColor, type, sampleCount);
		m_SRVDescriptor = Texture2DSRVDescriptorHandle::Create(m_Resource);
	}

	Texture2D::Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format) :
		Texture2D(width, height, mips, format, {0,0,0,0}, TextureType::Texture, MSAASampleCount::MSAA1)
	{}

	Texture2D::~Texture2D()
	{
		if (m_DataOwner)
		{
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
			Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		}

		m_SRVDescriptor = nullptr;
		m_Resource = nullptr;
	}

	void Texture2D::Resize(uint32 width, uint32 height)
	{
		if (!m_Resizeable)
		{
			CORE_WARN("Can not resize texture");
			return;
		}

		// schedule old resource destruction
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);

		// create new resource
		m_Resource = Texture2DResource::Create(
			width, height, m_Resource->GetMips(), 
			m_Resource->GetFormat(), 
			m_Resource->GetClearColor(), 
			m_Resource->GetTextureType(), 
			m_Resource->GetMSAASampleCount()
		);

		// rebind srv handles
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		m_SRVDescriptor = Texture2DSRVDescriptorHandle::Create(m_Resource);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height)
	{
		return Create(width, height, 0, TextureFormat::RGBA8_UNORM);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, uint32 mips)
	{
		return Create(width, height, mips, TextureFormat::RGBA8_UNORM);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, TextureFormat format)
	{
		return Create(width, height, 0, format);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		return CreateRef<Texture2D>(width, height, mips, format);
	}

	Ref<Texture2D> Texture2D::Create(const fs::path& path)
	{
		CORE_ASSERT(path != "", "Path must be given");
		TextureFile* file = TextureFile::LoadFile(path);
		//if(file->channels == 3) // no 3 component texture format exists
		file->ConvertToChannels(4);

		Ref<Texture2D> texture = Create(file->width, file->height, file->GetTextureFormat());
		texture->SetData(file->data);
		delete file;
		return texture;
	}

	bool Texture2D::ValidExtension(const fs::path& ext)
	{
		return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp");
	}



	RWTexture2D::RWTexture2D(Texture2DResource* resource, Texture2DSRVDescriptorHandle* srv, RenderTarget2D* owner) :
		Texture2D(resource, srv), m_Owner(owner)
	{
		GenerateUAVDescriptors();
	}

	RWTexture2D::RWTexture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor) :
		Texture2D(width, height, mips, format, clearColor, TextureType::RWTexture, MSAASampleCount::MSAA1), m_Owner(nullptr)
	{
		GenerateUAVDescriptors();
	}

	RWTexture2D::~RWTexture2D()
	{
		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptors[i]);
		m_UAVDescriptors.Clear();
	}

	void RWTexture2D::Resize(uint32 width, uint32 height)
	{
		if (!m_Resizeable)
		{
			CORE_WARN("Can not resize texture");
			return;
		}

		if (m_Owner)
			m_Owner->Resize(width, height);
		else
		{
			// resize the base texture
			Texture2D::Resize(width, height);
			GenerateUAVDescriptors();
		}

	}

	Ref<RWTexture2D> RWTexture2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		return Create(width, height, mips, format, { 0,0,0,0 });
	}

	Ref<RWTexture2D> RWTexture2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearClolor)
	{
		return CreateRef<RWTexture2D>(width, height, mips, format, clearClolor);
	}

	void RWTexture2D::GenerateUAVDescriptors()
	{
		uint32 width = m_Resource->GetWidth();
		uint32 height = m_Resource->GetHeight();

		for (uint32 i = 0; i < m_UAVDescriptors.Count(); i++)
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptors[i]);

		m_UAVDescriptors.Resize(m_Resource->GetMips());
		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
		{
			uint32 w = i == 0 ? width : width / (2 * i);
			uint32 h = i == 0 ? height : height / (2 * i);
			m_UAVDescriptors[i] = Texture2DUAVDescriptorHandle::Create(m_Resource, i, w, h);
		}
	}



	RenderTarget2D::RenderTarget2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, MSAASampleCount sampleCount, bool RWCapable) :
		Texture2D(width, height, mips, format, clearColor, (TextureType)((uint32)TextureType::RenderTarget | (uint32)(RWCapable ? TextureType::RWTexture : TextureType::Texture)), sampleCount)
	{
		m_RTVDSVDescriptor = Texture2DRTVDSVDescriptorHandle::Create(m_Resource);

		if (RWCapable)
			m_RWTexture = CreateRef<RWTexture2D>(m_Resource, m_SRVDescriptor, this);
	}

	RenderTarget2D::~RenderTarget2D()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_RTVDSVDescriptor);
		m_RTVDSVDescriptor = nullptr;

		m_RWTexture.reset();
	}

	void RenderTarget2D::Resize(uint32 width, uint32 height)
	{
		if (!m_Resizeable)
		{
			CORE_WARN("Can not resize texture");
			return;
		}

		// resize the base texture
		Texture2D::Resize(width, height);
		// rebind the rtv/dsv handle
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_RTVDSVDescriptor);
		m_RTVDSVDescriptor = Texture2DRTVDSVDescriptorHandle::Create(m_Resource);

		if (m_RWTexture)
		{
			m_RWTexture->m_Resource = m_Resource;
			m_RWTexture->m_SRVDescriptor = m_SRVDescriptor;
			m_RWTexture->GenerateUAVDescriptors();
		}
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format)
	{
		Description desc(width, height, format);
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, MSAASampleCount sampelCount)
	{
		Description desc(width, height, format);
		desc.sampleCount = sampelCount;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, MSAASampleCount sampelCount, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.sampleCount = sampelCount;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor)
	{
		Description desc(width, height, format);
		desc.clearColor = clearColor;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.clearColor = clearColor;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor, MSAASampleCount sampleCount)
	{
		Description desc(width, height, format);
		desc.clearColor = clearColor;
		desc.sampleCount = sampleCount;
		return Create(desc);
	}
	
	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, Math::Vector4 clearColor, MSAASampleCount sampleCount, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.clearColor = clearColor;
		desc.sampleCount = sampleCount;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, MSAASampleCount sampelCount)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.sampleCount = sampelCount;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, MSAASampleCount sampelCount, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.sampleCount = sampelCount;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.clearColor = clearColor;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.clearColor = clearColor;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, MSAASampleCount sampleCount)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.clearColor = clearColor;
		desc.sampleCount = sampleCount;
		return Create(desc);
	}
	
	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, MSAASampleCount sampleCount, bool RWCapable)
	{
		Description desc(width, height, format);
		desc.mips = mips;
		desc.clearColor = clearColor;
		desc.sampleCount = sampleCount;
		desc.RWCapable = RWCapable;
		return Create(desc);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(const Description& desc)
	{
		CORE_ASSERT(desc.width != 0 && desc.height != 0, "width and height cant be 0");
		CORE_ASSERT(!(IsTextureFormatDepthStencil(desc.format) && desc.RWCapable), "Depth Stencil formats can not be Read/Write capable");
		return CreateRef<RenderTarget2D>(desc.width, desc.height, desc.mips, desc.format, desc.clearColor, desc.sampleCount, desc.RWCapable);
	}

	// Utils ---------------------------------------------------------- //

	TextureFile::~TextureFile()
	{
		free(data);
	}

	TextureFormat TextureFile::GetTextureFormat()
	{
		if (HDR)
		{
			switch (channels)
			{
			case 1: return TextureFormat::R16_FLOAT;
			case 2: return TextureFormat::RG16_FLOAT;
			case 4: return TextureFormat::RGBA16_FLOAT;
			}
		}
		else
		{
			switch (channels)
			{
			case 1: return TextureFormat::R8_UNORM;
			case 2: return TextureFormat::RG8_UNORM;
			case 4: return TextureFormat::RGBA8_UNORM;
			}
		}
		return TextureFormat::None;
	}

	void TextureFile::ConvertToChannels(uint8 numChannels)
	{

		if (channels == numChannels)
			return;

		const uint16 fillval[] = { 0,0,0,255 };

		uint8 minChannels = channels < numChannels ? channels : numChannels;
		uint32 oldImageStrid = width * channels;
		uint32 newImageStrid = width * numChannels;

#define MAP_DATA(type){ type* newData = (type*)malloc(width * height * numChannels * sizeof(type)); \
		for (uint32 y = 0; y < height; y++){\
			for (uint32 x = 0; x < width; x++){\
				for (uint8 c = 0; c < numChannels; c++){\
					uint32 oldImageIndex = (oldImageStrid * y) + (x * channels) + c;\
					uint32 newImageIndex = (newImageStrid * y) + (x * numChannels) + c;\
					newData[newImageIndex] = (c < minChannels) ? ((type*)data)[oldImageIndex] : fillval[c];\
				}\
			}\
		}\
		free(data);\
		data = newData;}

		if (HDR) MAP_DATA(uint16)
		else MAP_DATA(uint8);

		channels = numChannels;

#undef MAP_DATA
	}

	TextureFile* TextureFile::LoadFile(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTIONI();

		TextureFile* texture = new TextureFile();
		int width, height, channels;
		int reqChannels = 0;

		if (file.extension() == ".hdr")
		{
			texture->HDR = true;
			texture->data = stbi_load_16(file.string().c_str(), &width, &height, &channels, reqChannels);
		}
		else
		{
			texture->HDR = false;
			texture->data = stbi_load(file.string().c_str(), &width, &height, &channels, reqChannels);
		}

		texture->width = width;
		texture->height = height;
		texture->channels = channels;

		CORE_ASSERT(texture->data, "Failed to load image \"{0}\"", file.string());

		return texture;
	}

	uint32 FixMipLevels(uint32 mips, uint32 width, uint32 height)
	{
		return mips == 0 ? (uint32)std::floor(std::log2(std::max(width, height))) + 1 : mips;
	}
}
