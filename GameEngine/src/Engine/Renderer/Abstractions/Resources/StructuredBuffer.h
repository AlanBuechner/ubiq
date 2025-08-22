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
		enum class ParentType
		{
			StructuredBuffer,
			VertexBuffer,
			IndexBuffer
		};

	public:
		virtual ~StructuredBufferResource() = 0;

		uint32 GetCount() { return m_Count; }
		uint32 GetStride() { return m_Stride; }
		ParentType GetParentType() { return m_ParentType; }

		virtual bool SupportState(ResourceState state) override;

		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) = 0;

		static StructuredBufferResource* Create(uint32 count, uint32 stride, ParentType parentType);

	protected:
		uint32 m_Count = 0;
		uint32 m_Stride = 0;

		ParentType m_ParentType;
	};

	// Descriptor Handles ---------------------------------------------------------- //

	class StructuredBufferSRVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void Bind(StructuredBufferResource* resource) = 0;

		static StructuredBufferSRVDescriptorHandle* Create(StructuredBufferResource* resource);
	};

	class StructuredBufferUAVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void Bind(StructuredBufferResource* resource) = 0;

		static StructuredBufferUAVDescriptorHandle* Create(StructuredBufferResource* resource);
	};


	// Structured Buffer Objects ---------------------------------------------------------- //

	class StructuredBuffer
	{
	protected:
		StructuredBuffer(StructuredBufferResource* resource, StructuredBufferSRVDescriptorHandle* srv);
		StructuredBuffer(uint32 count, uint32 stride, StructuredBufferResource::ParentType parantType);

	public:
		StructuredBuffer(uint32 count, uint32 stride);
		virtual ~StructuredBuffer();
		StructuredBufferResource* GetResource() { return m_Resource; }
		StructuredBufferSRVDescriptorHandle* GetSRVDescriptor() { return m_SRVDescriptor; }

		uint32 GetCount() { return m_Resource->GetCount(); }
		uint32 GetStride() { return m_Resource->GetStride(); }

		virtual void Resize(uint32 count);
		void SetData(const void* data, uint32 count = 1, uint32 start = 0) { m_Resource->SetData(data, count, start); }
		template<typename T>
		void SetData(Utils::Vector<T> data) { SetData(data.Data(), data.Count()); }

		static Ref<StructuredBuffer> Create(uint32 count, uint32 stride);

	protected:
		StructuredBufferResource* m_Resource;
		StructuredBufferSRVDescriptorHandle* m_SRVDescriptor;
		bool m_DataOwner = true;
	};

	class RWStructuredBuffer : public StructuredBuffer
	{
	public:
		RWStructuredBuffer(StructuredBufferResource* resource, StructuredBufferSRVDescriptorHandle* srv, StructuredBuffer* owner);
		RWStructuredBuffer(uint32 count, uint32 stride);
		virtual ~RWStructuredBuffer();

		StructuredBufferUAVDescriptorHandle* GetUAVDescriptor() { return m_UAVDescriptor; }

		virtual void Resize(uint32 count) override;

		void GenerateUAVDescriptor();

		static Ref<RWStructuredBuffer> Create(uint32 count, uint32 stride);

	protected:
		StructuredBufferUAVDescriptorHandle* m_UAVDescriptor = nullptr;
		StructuredBuffer* m_Owner = nullptr;

		friend class VertexBuffer;
		friend class IndexBuffer;
	};
}
