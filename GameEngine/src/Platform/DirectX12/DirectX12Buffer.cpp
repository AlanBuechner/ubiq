#include "pch.h"
#include "DirectX12Buffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	DirectX12VertexBuffer::DirectX12VertexBuffer(uint32 size, uint32 stride) :
		m_Stride(stride)
	{
		CreateBuffer(size);
	}

	DirectX12VertexBuffer::DirectX12VertexBuffer(const void* vertices, uint32 size, uint32 stride) :
		m_Stride(stride)
	{
		SetData(vertices, size);
	}

	DirectX12VertexBuffer::~DirectX12VertexBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12VertexBuffer::SetData(const void* data, uint32 count)
	{
		if (m_Count != count)
			CreateBuffer(count);

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, count * m_Stride, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void DirectX12VertexBuffer::CreateBuffer(uint32 count)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_Buffer)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);

		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(count * m_Stride), // resource description for a buffer
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);

		m_View.BufferLocation = m_Buffer->GetGPUVirtualAddress();
		m_View.StrideInBytes = m_Stride;
		m_View.SizeInBytes = count * m_Stride;

		m_Count = count;
	}










	// IndexBuffer ---------------------------------------------------------------------------------------

	DirectX12IndexBuffer::DirectX12IndexBuffer(uint32 count)
	{
		CreateBuffer(count);
	}

	DirectX12IndexBuffer::DirectX12IndexBuffer(const uint32* indices, uint32 count)
	{
		SetData(indices, count);
	}

	DirectX12IndexBuffer::~DirectX12IndexBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12IndexBuffer::SetData(const uint32* data, uint32 count)
	{
		if (m_Count != count)
			CreateBuffer(count);

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, count * sizeof(uint32), D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	void DirectX12IndexBuffer::CreateBuffer(uint32 count)
	{
		if (count == 0)
			return;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_Buffer)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);

		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(uint32)), // resource description for a buffer
			D3D12_RESOURCE_STATE_INDEX_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);

		m_View.BufferLocation = m_Buffer->GetGPUVirtualAddress();
		m_View.SizeInBytes = count * sizeof(uint32);
		m_View.Format = DXGI_FORMAT_R32_UINT;
		
		m_Count = count;
	}

}
