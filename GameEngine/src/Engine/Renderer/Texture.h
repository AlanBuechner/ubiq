#pragma once
#include "Engine/Core/core.h"
#include "TextureAttribute.h"
#include <glm/glm.hpp>
#include <string>

namespace Engine
{
	class Texture
	{

	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void Bind(uint32_t slot) const = 0;
		virtual void LoadFromFile(const std::string& path) = 0;
		virtual Ref<TextureAttribute> GetAttributes() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path = "", TextureAttribute& attribute = *TextureAttribute::Default);
		static Ref<Texture2D> Create(const uint32_t width, const uint32_t height, TextureAttribute& attribute = *TextureAttribute::Default);

	};
}