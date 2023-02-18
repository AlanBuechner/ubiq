#pragma once

namespace Engine
{
	struct TextureAttribute
	{
		enum class WrapMode
		{
			Repeat,
			MirroredRepeat,
			Clamp
		};

		enum class MinMagFilter
		{
			Point,
			Linear,
			Anisotropic
		};

		WrapMode U = WrapMode::Repeat, V = WrapMode::Repeat;

		MinMagFilter Min = MinMagFilter::Linear, Mag = MinMagFilter::Linear;

		static TextureAttribute* Default;
	};
}
