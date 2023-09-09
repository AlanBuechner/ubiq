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

	Texture2DResource* Texture2DResource::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12Texture2DResource(width, height, mips, format);
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
			handle->ReBind(resource);
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
			handle->ReBind(resource);
		return handle;
	}



	// Texture Objects ---------------------------------------------------------- //

	Texture2D::Texture2D(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		m_Resource = Texture2DResource::Create(width, height, mips, format);

		m_SRVDescriptor = Texture2DSRVDescriptorHandle::Create(m_Resource);
	}

	Texture2D::~Texture2D()
	{
		CORE_INFO("Deleteing Texture : {0}", (uint64)m_Resource->GetGPUResourcePointer());
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
	}

	void Texture2D::Resize(uint32 width, uint32 height)
	{
		// schedule old resource destruction
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);

		Texture2DResource** p = &m_Resource;

		// create new resource
		uint32 oldWidth = m_Resource->GetWidth();
		uint32 oldHeight = m_Resource->GetHeight();
		uint64 oldPointer = (uint64)m_Resource->GetGPUResourcePointer();
		m_Resource = Texture2DResource::Create(width, height, m_Resource->GetMips(), m_Resource->GetFormat());
		CORE_INFO("Resizeing Texture : {0}({1}, {2}) -> {3}({4}, {5})", oldPointer, oldWidth, oldHeight, (uint64)m_Resource->GetGPUResourcePointer(), m_Resource->GetWidth(), m_Resource->GetHeight());

		// rebind srv handles
		m_SRVDescriptor->ReBind(m_Resource);
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

		Ref<Texture2D> texture = Create(file->width, file->height, file->GetTextureFormat());
		texture->SetData(file->data);
		delete file;
		return texture;
	}

	bool Texture2D::ValidExtension(const fs::path& ext)
	{
		return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp");
	}




	RenderTarget2D::RenderTarget2D(uint32 width, uint32 height, uint32 mips, TextureFormat format) :
		Texture2D(width, height, mips, format)
	{
		m_RTVDSVDescriptor = Texture2DRTVDSVDescriptorHandle::Create(m_Resource);
	}

	RenderTarget2D::~RenderTarget2D()
	{
		Texture2D::~Texture2D();
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_RTVDSVDescriptor);
	}

	void RenderTarget2D::Resize(uint32 width, uint32 height)
	{
		// resize the base texture
		Texture2D::Resize(width, height);

		// rebind the rtv/dsv handle
		m_RTVDSVDescriptor->ReBind(m_Resource);
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		return CreateRef<RenderTarget2D>(width, height, mips, format);
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
			case 2: return TextureFormat::None; // TODO : add in new texture format
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
#define MAP_DATA(type){ type* newData = new type[width * height * channels]; \
		for (uint32 x = 0; x < width; x++){\
			for (uint32 y = 0; y < height; y++){\
				for (uint8 c = 0; c < numChannels; c++){\
					uint32 oldImageIndex = (oldImageStrid * y) + (x * channels) + c;\
					uint32 newImageIndex = (newImageStrid * y) + (x * channels) + c;\
					newData[newImageIndex] = (c < minChannels) ? ((type*)data)[oldImageIndex] : 0;\
				}\
			}\
		}\
		delete[] data;\
		data = newData;}

		if (channels == numChannels)
			return;

		uint8 minChannels = channels < numChannels ? channels : numChannels;
		uint8 oldImageStrid = width * channels;
		uint8 newImageStrid = width * numChannels;
		if (HDR)
			MAP_DATA(uint16)
		else
			MAP_DATA(uint8)

			channels = numChannels;
	}

	TextureFile* TextureFile::LoadFile(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTION();

		TextureFile* texture = new TextureFile();
		int width, height, channels;

		if (file.extension() == ".hdr")
		{
			texture->HDR = true;
			texture->data = stbi_load_16(file.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		}
		else
		{
			texture->HDR = false;
			texture->data = stbi_load(file.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		}

		texture->width = width;
		texture->height = height;
		texture->channels = channels;

		CORE_ASSERT(texture->data, "Failed to load image \"{0}\"", file.string());

		return texture;
	}
}
