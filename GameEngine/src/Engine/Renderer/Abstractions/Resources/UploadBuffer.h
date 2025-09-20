#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"
#include "TextureFormat.h"

namespace Engine
{
	class UploadBufferResource : public GPUResource
	{
	public:
		virtual ~UploadBufferResource() = 0;

		uint32 GetSize() { return m_Size; }
		void* GetMemory() { return m_MapedBasePointer; }

		virtual void* Map() = 0;
		virtual void UnMap() = 0;

		static UploadBufferResource* Create(uint32 size);

	protected:
		virtual bool SupportState(ResourceState state) const override;

	protected:
		void* m_MapedBasePointer = nullptr;
		uint32 m_Size;

	};





	class UploadTextureResource : public GPUResource
	{
	public:
		virtual ~UploadTextureResource() = 0;

		uint32 GetWidth() { return m_Width; }
		uint32 GetHeight() { return m_Height; }
		uint32 GetMips() { return m_Mips; }
		TextureFormat GetFormat() { return m_Format; }
		uint32 GetStride() { return GetTextureFormatStride(m_Format); }
		uint32 GetPitch() { return m_Pitch; }
		uint32 GetUploadSize() { return m_Height * m_Pitch; }
		void* GetMemory() { return m_MapedBasePointer; }

		virtual void* Map() = 0;
		virtual void UnMap() = 0;

		static UploadTextureResource* Create(uint32 width, uint32 height, uint32 mips, TextureFormat format);

	protected:
		virtual bool SupportState(ResourceState state) const override;

	protected:
		void* m_MapedBasePointer = nullptr;
		uint32 m_Width, m_Height, m_Mips;
		uint32 m_Pitch;
		TextureFormat m_Format;

	};

}
