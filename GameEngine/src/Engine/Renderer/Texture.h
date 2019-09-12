#pragma once
#include "Engine/core.h"
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
		virtual void Bind(uint32_t slot) const = 0;
		virtual void LoadFromFile(const std::string& path) = 0;
		virtual Ref<TextureAttribute> GetAttributes() const = 0;
		virtual glm::vec2 AtlasIndexToPosition(int index) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path = "", TextureAttribute& attribute = *TextureAttribute::Default);
	};
}