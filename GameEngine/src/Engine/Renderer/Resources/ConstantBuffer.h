#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"

namespace Engine
{
	class ConstantBufferResource : public GPUResource
	{
	public:
		virtual ~ConstantBufferResource() = 0;

		uint32 GetSize() { return m_Size; }

		virtual bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Size = 0;
	};
	

	class ConstantBuffer
	{
	public:
		
		virtual void SetData(const void* data) = 0;

		virtual uint32 GetDescriptorLocation() = 0;

		virtual Ref<ConstantBufferResource> GetResource() = 0;

		static Ref<ConstantBuffer> Create(uint32 size);
		static Ref<ConstantBuffer> Create(Ref<ConstantBufferResource> resource);
	};

	// idk if this is a thing but everything else has a read write variant so ill just leave this here for now
	class RWConstantBuffer
	{
	public:

		virtual void SetData(const void* data) = 0;

		virtual uint32 GetDescriptorLocation() = 0;

		virtual Ref<ConstantBufferResource> GetResource() = 0;

		static Ref<RWConstantBuffer> Create(uint32 size);
		static Ref<RWConstantBuffer> Create(Ref<ConstantBufferResource> resource);
	};
}
