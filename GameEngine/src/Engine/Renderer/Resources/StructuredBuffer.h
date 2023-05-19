#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"

namespace Engine
{
	class StructuredBufferResource : public GPUResource
	{
	public:
		virtual ~StructuredBufferResource() = 0;

		uint32 GetStride() { return m_Stride; }
		uint32 GetCount() { return m_Count; }

		virtual bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Stride = 0;
		uint32 m_Count = 0;
	};

	class StructuredBuffer
	{
	public:
		virtual void Resize(uint32 count) = 0;
		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) = 0;

		virtual uint32 GetDescriptorLocation() const = 0;

		virtual Ref<StructuredBufferResource> GetResource() = 0;

		static Ref<StructuredBuffer> Create(uint32 stride, uint32 count);
		static Ref<StructuredBuffer> Create(Ref<StructuredBufferResource> resource);
	};

	class RWStructuredBuffer
	{
	public:
		virtual void Resize(uint32 count) = 0;
		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) = 0;

		virtual uint32 GetDescriptorLocation() const = 0;

		virtual Ref<StructuredBufferResource> GetResource() = 0;

		static Ref<RWStructuredBuffer> Create(uint32 stride, uint32 count);
		static Ref<RWStructuredBuffer> Create(Ref<StructuredBufferResource> resource);
	};
}
