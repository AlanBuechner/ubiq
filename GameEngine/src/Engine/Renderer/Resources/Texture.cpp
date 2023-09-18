#include "pch.h"
#include "Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Texture.h"

#include "stb_image.h"

namespace Engine
{

	// Texture Resource ---------------------------------------------------------- //

	bool IsDepthStencil(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::DEPTH24STENCIL8:
			return true;
		}
		return false;
	}

	Texture2DResource::~Texture2DResource() {}


	uint32 Texture2DResource::GetStride()
	{
		switch (m_Format)
		{
		case TextureFormat::RGBA8:				return 4*1;
		case TextureFormat::RGBA16:				return 4*2;
		case TextureFormat::RGBA32:				return 4*4;
		case TextureFormat::RG16:				return 2*2;
		case TextureFormat::RG32:				return 2*4;
		case TextureFormat::RED_INTEGER:		return 4*1;
		case TextureFormat::DEPTH24STENCIL8:	return 24+8;
		}
		return 0;
	}

	Texture2DResource* Texture2DResource::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12Texture2DResource(width, height, mips, format, clearColor, type);
		}
		return nullptr;
	}

	bool Texture2DResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::Common:
		case ResourceState::ShaderResource:
		case ResourceState::UnorderedResource:
		case ResourceState::RenderTarget:
			return true;
		}
		return false;
	}

	// Descriptor Handles ---------------------------------------------------------- //

	Texture2DSRVDescriptorHandle* Texture2DSRVDescriptorHandle::Create(Texture2DResource* resource)
	{
		Texture2DSRVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12Texture2DSRVDescriptorHandle();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}


	Texture2DUAVDescriptorHandle* Texture2DUAVDescriptorHandle::Create(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height)
	{
		Texture2DUAVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12Texture2DUAVDescriptorHandle();
			break;
		}

		if (handle)
			handle->Bind(resource, mipSlice, width, height);
		return handle;
	}



	Texture2DRTVDSVDescriptorHandle* Texture2DRTVDSVDescriptorHandle::Create(Texture2DResource* resource)
	{
		Texture2DRTVDSVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12Texture2DRTVDSVDescriptorHandle(resource);
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}



	// Texture Objects ---------------------------------------------------------- //

	Texture2D::Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, TextureType type)
	{
		m_Resource = Texture2DResource::Create(width, height, mips, format, clearColor, type);

		m_SRVDescriptor = Texture2DSRVDescriptorHandle::Create(m_Resource);
	}

	Texture2D::Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format) :
		Texture2D(width, height, mips, format, {0,0,0,0}, TextureType::Texture)
	{}


	Texture2D::~Texture2D()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		m_SRVDescriptor = nullptr;

		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
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
		m_Resource = Texture2DResource::Create(width, height, m_Resource->GetMips(), m_Resource->GetFormat(), m_Resource->GetClearColor(), m_Resource->GetTextureType());

		// rebind srv handles
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		m_SRVDescriptor = Texture2DSRVDescriptorHandle::Create(m_Resource);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height)
	{
		return Create(width, height, 0, TextureFormat::RGBA8);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, uint32 mips)
	{
		return Create(width, height, mips, TextureFormat::RGBA8);
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



	RWTexture2D::RWTexture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor) :
		Texture2D(width, height, mips, format, clearColor, TextureType::RWTexture)
	{
		m_UAVDescriptors.resize(m_Resource->GetMips());
		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
		{
			uint32 w = i == 0 ? width : width / (2 * i);
			uint32 h = i == 0 ? height : height / (2 * i);
			m_UAVDescriptors[i] = Texture2DUAVDescriptorHandle::Create(m_Resource, i, w, h);
		}
	}

	RWTexture2D::~RWTexture2D()
	{
		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptors[i]);
		m_UAVDescriptors.clear();
	}

	void RWTexture2D::Resize(uint32 width, uint32 height)
	{
		if (!m_Resizeable)
		{
			CORE_WARN("Can not resize texture");
			return;
		}

		// resize the base texture
		Texture2D::Resize(width, height);

		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptors[i]);

		m_UAVDescriptors.resize(m_Resource->GetMips());
		for (uint32 i = 0; i < m_Resource->GetMips(); i++)
		{
			uint32 w = i == 0 ? width : width / (2 * i);
			uint32 h = i == 0 ? height : height / (2 * i);
			m_UAVDescriptors[i] = Texture2DUAVDescriptorHandle::Create(m_Resource, i, w, h);
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



	RenderTarget2D::RenderTarget2D(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, bool RWCapable) :
		Texture2D(width, height, mips, format, clearColor, (TextureType)((uint32)TextureType::RenderTarget | (RWCapable ? (uint32)TextureType::RWTexture : 0)))
	{
		m_RTVDSVDescriptor = Texture2DRTVDSVDescriptorHandle::Create(m_Resource);
	}

	RenderTarget2D::~RenderTarget2D()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_RTVDSVDescriptor);
		m_RTVDSVDescriptor = nullptr;
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
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format)
	{
		return Create(width, height, 1, format);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, TextureFormat format, bool RWCapable)
	{
		return Create(width, height, 1, format, {0,0,0,0}, RWCapable);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		return Create(width, height, mips, format, { 0,0,0,0 });
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor)
	{
		return Create(width, height, mips, format, clearColor, false);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format, Math::Vector4 clearColor, bool RWCapable)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		return CreateRef<RenderTarget2D>(width, height, mips, format, clearColor, RWCapable);
	}

	// Utils ---------------------------------------------------------- //

	TextureFile::~TextureFile()
	{
		delete[] data;
	}

	TextureFormat TextureFile::GetTextureFormat()
	{
		if (HDR)
		{
			switch (channels)
			{
			case 1: return TextureFormat::None; // TODO : add in new texture format
			case 2: return TextureFormat::RG16; // TODO : add in new texture format
			case 3: return TextureFormat::None; // TODO : add in new texture format
			case 4: return TextureFormat::RGBA16;
			}
		}
		else
		{
			switch (channels)
			{
			case 1: return TextureFormat::None; // TODO : add in new texture format
			case 2: return TextureFormat::None; // TODO : add in new texture format
			case 3: return TextureFormat::None; // TODO : add in new texture format
			case 4: return TextureFormat::RGBA8;
			}
		}
	}

	void TextureFile::ConvertToChannels(uint8 numChannels)
	{

		if (channels == numChannels)
			return;

		const uint16 fillval[] = { 0,0,0,255 };

		uint8 minChannels = channels < numChannels ? channels : numChannels;
		uint32 oldImageStrid = width * channels;
		uint32 newImageStrid = width * numChannels;

#define MAP_DATA(type){ type* newData = new type[width * height * numChannels]; \
		for (uint32 y = 0; y < height; y++){\
			for (uint32 x = 0; x < width; x++){\
				for (uint8 c = 0; c < numChannels; c++){\
					uint32 oldImageIndex = (oldImageStrid * y) + (x * channels) + c;\
					uint32 newImageIndex = (newImageStrid * y) + (x * numChannels) + c;\
					newData[newImageIndex] = (c < minChannels) ? ((type*)data)[oldImageIndex] : fillval[c];\
				}\
			}\
		}\
		delete[] data;\
		data = newData;}

		if (HDR) MAP_DATA(uint16)
		else MAP_DATA(uint8);

		channels = numChannels;

#undef MAP_DATA
	}

	TextureFile* TextureFile::LoadFile(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTION();

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
