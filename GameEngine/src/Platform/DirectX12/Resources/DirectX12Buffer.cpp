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

	DirectX12VertexBufferView::DirectX12VertexBufferView(StructuredBufferResource* resource) :
		VertexBufferView(resource)
	{}

	void DirectX12VertexBufferView::Bind()
	{
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)m_Resource;

		// check if resource has been allocated
		if (dxResource->GetBuffer() == nullptr)
			return;

		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.StrideInBytes = dxResource->GetStride();
		m_View.SizeInBytes = dxResource->GetCount() * dxResource->GetStride();
	}









	// IndexBuffer ---------------------------------------------------------------------------------------

	DirectX12IndexBufferView::DirectX12IndexBufferView(StructuredBufferResource* resource) :
		IndexBufferView(resource)
	{}

	void DirectX12IndexBufferView::Bind()
	{
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)m_Resource;

		// check if resource has been allocated
		if (dxResource->GetBuffer() == nullptr)
			return;

		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.SizeInBytes = dxResource->GetCount() * sizeof(uint32);
		m_View.Format = DXGI_FORMAT_R32_UINT;
	}

}
