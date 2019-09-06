#pragma once

namespace Engine
{
	struct TextureAttribute
	{
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

		WrapMode U = WrapMode::Repeat, V = WrapMode::Repeat;

		MinMagFilter Min = MinMagFilter::Linear, Mag = MinMagFilter::Linear;

		static TextureAttribute* Default;
	};
}