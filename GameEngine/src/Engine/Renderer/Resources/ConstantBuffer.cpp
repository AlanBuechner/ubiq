#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include "Platform/DirectX12/DirectX12ConstantBUffer.h"

namespace Engine
{

	// Constant Buffer Resource ---------------------------------------------------------- //

	ConstantBufferResource::~ConstantBufferResource() {}

	bool ConstantBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
		case ResourceState::UnorderedResource:
			return true;
		case ResourceState::Common:
		case ResourceState::RenderTarget:
			return false;
		}
	}

	ConstantBufferResource* ConstantBufferResource::Create(uint32 size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12ConstantBufferResource(size);
		}
		return nullptr;
	}

	// Descriptor Handles ---------------------------------------------------------- //


	ConstantBufferCBVDescriptorHandle* ConstantBufferCBVDescriptorHandle::Create(ConstantBufferResource* resource)
	{
		ConstantBufferCBVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12ConstantBufferCBVDescriptorHandle();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}


	// Constant Buffer Objects ---------------------------------------------------------- //

	ConstantBuffer::ConstantBuffer(uint32 size)
	{
		m_Resource = ConstantBufferResource::Create(size);
		m_CBVDesciptor = ConstantBufferCBVDescriptorHandle::Create(m_Resource);
	}

	ConstantBuffer::~ConstantBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_CBVDesciptor);
	}

	Ref<ConstantBuffer> ConstantBuffer::Create(uint32 size)
	{
		return CreateRef<ConstantBuffer>(size);
	}

}
