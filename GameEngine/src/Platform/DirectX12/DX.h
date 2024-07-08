#pragma once
#include "Platform/Windows/Win.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "d3dx12/d3dx12.h"
#include "d3d12.h"
#include <dxgi1_6.h>
#include <wrl.h>
namespace wrl = Microsoft::WRL;
