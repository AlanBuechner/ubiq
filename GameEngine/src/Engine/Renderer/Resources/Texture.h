#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Math/Math.h"
#include "Engine/AssetManager/AssetManager.h"

namespace Engine
{

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

	class Texture2DResource
	{
	public:
		virtual ~Texture2DResource() = 0;

		uint32 GetWidth() { return m_Width; }
		uint32 GetHeight() { return m_Height; }
		TextureFormat GetFormat() { return m_Format; }

	protected:
		uint32 m_Width = 1;
		uint32 m_Height = 1;

		TextureFormat m_Format = TextureFormat::RGBA8;

	};

	class Texture2D : public Asset
	{
	public:
		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
		virtual void* GetTextureHandle() const = 0;
		virtual uint32 GetDescriptorLocation() const = 0;

		virtual void SetData(void* data) = 0;
		virtual void LoadFromFile(const fs::path& path) = 0;

		virtual Ref<Texture2DResource> GetResource() = 0;

		virtual bool operator==(const Texture2D& other) const = 0;

		static Ref<Texture2D> Create(const fs::path& path = "");
		static Ref<Texture2D> Create(const uint32 width, const uint32 height);
		static Ref<Texture2D> Create(Ref<Texture2DResource> resource);

		static bool ValidExtension(const fs::path& ext);

	};
}
