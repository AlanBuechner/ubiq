#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	enum class TextureFormat
	{
		None = 0,

		// 8 bit components normalized
		R8_UNORM,
		RG8_UNORM,
		RGBA8_UNORM,
		R8_SNORM,
		RG8_SNORM,
		RGBA8_SNORM,

		// 8 bit components int
		R8_UINT,
		RG8_UINT,
		RGBA8_UINT,
		R8_SINT,
		RG8_SINT,
		RGBA8_SINT,

		// 16 bit components normalized
		R16_UNORM,
		RG16_UNORM,
		RGBA16_UNORM,
		R16_SNORM,
		RG16_SNORM,
		RGBA16_SNORM,

		// 16 bit components int
		R16_UINT,
		RG16_UINT,
		RGBA16_UINT,
		R16_SINT,
		RG16_SINT,
		RGBA16_SINT,

		// 16 bit components float
		R16_FLOAT,
		RG16_FLOAT,
		RGBA16_FLOAT,

		// 32 bit components int
		R32_UINT,
		RG32_UINT,
		RGBA32_UINT,
		R32_SINT,
		RG32_SINT,
		RGBA32_SINT,

		// 32 bit components float
		R32_FLOAT,
		RG32_FLOAT,
		RGBA32_FLOAT,

		// Depth/Stencil (only used for frame buffers)
		DEPTH24STENCIL8,

		Depth,
	};
	bool IsTextureFormatDepthStencil(TextureFormat format);
	uint32 GetTextureFormatStride(TextureFormat format);
}
