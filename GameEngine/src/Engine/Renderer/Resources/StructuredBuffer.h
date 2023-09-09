#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"

namespace Engine
{
	// Structured Buffer Resource ---------------------------------------------------------- //

	class StructuredBufferResource : public GPUResource
	{
	public:
		virtual ~StructuredBufferResource() = 0;

		uint32 GetStride() { return m_Stride; }
		uint32 GetCount() { return m_Count; }

		virtual bool SupportState(ResourceState state) override;

		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) = 0;

		static StructuredBufferResource* Create(uint32 stride, uint32 count);

	protected:
		uint32 m_Stride = 0;
		uint32 m_Count = 0;
	};

	// Descriptor Handles ---------------------------------------------------------- //

	class StructuredBufferSRVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void ReBind(StructuredBufferResource* resource) = 0;

		static StructuredBufferSRVDescriptorHandle* Create(StructuredBufferResource* resource);
	};

	class StructuredBufferUAVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void ReBind(StructuredBufferResource* resource) = 0;

		static StructuredBufferUAVDescriptorHandle* Create(StructuredBufferResource* resource);
	};


	// Structured Buffer Objects ---------------------------------------------------------- //

	class StructuredBuffer
	{
	public:
		StructuredBuffer(uint32 stride, uint32 count);
		virtual ~StructuredBuffer();
		StructuredBufferResource* GetResource() { return m_Resource; }
		StructuredBufferSRVDescriptorHandle* GetSRVDescriptor() { return m_SRVDescriptor; }

		virtual void Resize(uint32 count);
		void SetData(void* data, uint32 count = 1, uint32 start = 0) { m_Resource->SetData(data, count, start); }

		static Ref<StructuredBuffer> Create(uint32 stride, uint32 count);

	protected:
		StructuredBufferResource* m_Resource;
		StructuredBufferSRVDescriptorHandle* m_SRVDescriptor;
	};

	class RWStructuredBuffer : public StructuredBuffer
	{
	public:
		RWStructuredBuffer(uint32 stride, uint32 count);
		virtual ~RWStructuredBuffer();

		StructuredBufferUAVDescriptorHandle* GetUAVDescriptor() { return m_UAVDescriptor; }

		virtual void Resize(uint32 count) override;

		static Ref<RWStructuredBuffer> Create(uint32 stride, uint32 count);

	protected:
		StructuredBufferUAVDescriptorHandle* m_UAVDescriptor;
	};
}
