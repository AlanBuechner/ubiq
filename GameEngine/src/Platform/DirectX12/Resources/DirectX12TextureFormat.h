#pragma once
#include "Engine/Renderer/Abstractions/Resources/TextureFormat.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	DXGI_FORMAT GetDXGITextureFormat(TextureFormat format);
	DXGI_FORMAT GetDXGISRVTextureFormat(TextureFormat format);
}
