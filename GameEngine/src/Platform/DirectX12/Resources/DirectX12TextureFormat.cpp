#include "DirectX12TextureFormat.h"



namespace Engine
{
	DXGI_FORMAT GetDXGITextureFormat(TextureFormat format)
	{
#define FORMAT_1(bit, type) case TextureFormat::R##bit##_##type: return DXGI_FORMAT_R##bit##_##type
#define FORMAT_2(bit, type) case TextureFormat::RG##bit##_##type: return DXGI_FORMAT_R##bit##G##bit##_##type
#define FORMAT_4(bit, type) case TextureFormat::RGBA##bit##_##type: return DXGI_FORMAT_R##bit##G##bit##B##bit##A##bit##_##type
		switch (format)
		{
			// 8 bit components normalized
			FORMAT_1(8, UNORM);
			FORMAT_2(8, UNORM);
			FORMAT_4(8, UNORM);
			FORMAT_1(8, SNORM);
			FORMAT_2(8, SNORM);
			FORMAT_4(8, SNORM);

			// 8 bit components int
			FORMAT_1(8, UINT);
			FORMAT_2(8, UINT);
			FORMAT_4(8, UINT);
			FORMAT_1(8, SINT);
			FORMAT_2(8, SINT);
			FORMAT_4(8, SINT);

			// 16 bit components normalized
			FORMAT_1(16, UNORM);
			FORMAT_2(16, UNORM);
			FORMAT_4(16, UNORM);
			FORMAT_1(16, SNORM);
			FORMAT_2(16, SNORM);
			FORMAT_4(16, SNORM);

			// 16 bit components int
			FORMAT_1(16, UINT);
			FORMAT_2(16, UINT);
			FORMAT_4(16, UINT);
			FORMAT_1(16, SINT);
			FORMAT_2(16, SINT);
			FORMAT_4(16, SINT);

			// 16 bit components float
			FORMAT_1(16, FLOAT);
			FORMAT_2(16, FLOAT);
			FORMAT_4(16, FLOAT);


			// 32 bit components int
			FORMAT_1(32, UINT);
			FORMAT_2(32, UINT);
			FORMAT_4(32, UINT);
			FORMAT_1(32, SINT);
			FORMAT_2(32, SINT);
			FORMAT_4(32, SINT);

			// 32 bit components float
			FORMAT_1(32, FLOAT);
			FORMAT_2(32, FLOAT);
			FORMAT_4(32, FLOAT);


		case TextureFormat::DEPTH24STENCIL8:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default: return DXGI_FORMAT_UNKNOWN;
		}
		
#undef FORMAT_1
#undef FORMAT_2
#undef FORMAT_4
	}

	DXGI_FORMAT GetDXGISRVTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::DEPTH24STENCIL8:	return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default: return GetDXGITextureFormat(format);
		}
	}
}
