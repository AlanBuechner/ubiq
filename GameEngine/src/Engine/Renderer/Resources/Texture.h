#pragma once
#include "Engine/Core/Core.h"
#include "TextureAttribute.h"
#include "Engine/Math/Math.h"
#include "Engine/AssetManager/AssetManager.h"

namespace Engine
{
	class Texture : public Asset
	{

	public:
		virtual ~Texture() = default;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
		virtual void* GetTextureHandle() const = 0;
		virtual void* GetSamplerHandle() const = 0;
		virtual uint32 GetDescriptorLocation() const = 0;

		virtual void SetData(void* data) = 0;
		virtual void LoadFromFile(const fs::path& path) = 0;
		virtual Ref<TextureAttribute> GetAttributes() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const fs::path& path = "", TextureAttribute& attribute = *TextureAttribute::Default);
		static Ref<Texture2D> Create(const uint32 width, const uint32 height, TextureAttribute& attribute = *TextureAttribute::Default);

		static bool ValidExtention(const fs::path& ext);

	};
}
