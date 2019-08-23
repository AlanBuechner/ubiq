#pragma once
#include "Engine/core.h"
#include <string>

namespace Engine
{
	class Texture
	{
	public:
		enum class WrapMode
		{
			Repeat,
			MirroredRepeat,
			ClampToEdge
		};

		enum class MinMagFilter
		{
			Nearest,
			Linear
		};

	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Bind(uint32_t slot) const = 0;
		virtual void SetWrapMode(WrapMode U, WrapMode V) = 0;
		virtual void SetMinMagFilter(MinMagFilter min, MinMagFilter mag) = 0;
		virtual void LoadFromFile(const std::string& path) = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path = "");
	};
}