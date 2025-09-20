#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12ConstantBuffer.h"
#endif

namespace Engine
{

	// Constant Buffer Resource ---------------------------------------------------------- //

	ConstantBufferResource::~ConstantBufferResource() {}

	bool ConstantBufferResource::SupportState(ResourceState state) const
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
		case ResourceState::UnorderedResource:
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		default: return false;
		}
	}

	ConstantBufferResource* ConstantBufferResource::Create(uint32 size)
	{
		CREATE_PROFILE_SCOPEI("Create Constant Buffer Resource");
		ANOTATE_PROFILEI(std::to_string(size) + " bytes");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12ConstantBufferResource(size);
		default: return nullptr;
		}
	}

	// Descriptor Handles ---------------------------------------------------------- //


	ConstantBufferCBVDescriptorHandle* ConstantBufferCBVDescriptorHandle::Create(ConstantBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			ConstantBufferCBVDescriptorHandle* handle = new DirectX12ConstantBufferCBVDescriptorHandle(resource);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
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
