#include "pch.h"
#include "DirectX12Buffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"
#include "DirectX12StructuredBuffer.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	void DirectX12VertexBufferView::Bind(StructuredBufferResource* resource)
	{
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)resource;
		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.StrideInBytes = dxResource->GetStride();
		m_View.SizeInBytes = dxResource->GetCount() * dxResource->GetStride();
	}








	// IndexBuffer ---------------------------------------------------------------------------------------

	void DirectX12IndexBufferView::Bind(StructuredBufferResource* resource)
	{
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)resource;
		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.SizeInBytes = dxResource->GetCount() * sizeof(uint32);
		m_View.Format = DXGI_FORMAT_R32_UINT;
	}

}
