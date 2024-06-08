#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"
#include "Engine/Math/Math.h"

namespace Engine
{

	class InstanceBufferResource : public GPUResource
	{
	public:
		virtual ~InstanceBufferResource() = 0;

		uint32 GetStride() { return m_Stride; }
		uint32 GetCapacity() { return m_Capacity; }

		virtual void SetData(const void* data, uint32 count) = 0;

		static InstanceBufferResource* Create(uint32 capacity, uint32 stride);

	protected:
		bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Stride;
		uint32 m_Capacity;
	};

	// instance buffers don't use descriptors they use views but architecturally they are similar
	class InstanceBufferView : public Descriptor
	{
	public:
		virtual void Bind(InstanceBufferResource* resource) = 0;

		static InstanceBufferView* Create(InstanceBufferResource* resource);
	};

	class InstanceBuffer
	{
	public:
		InstanceBuffer(uint32 capacity, uint32 stride);
		~InstanceBuffer();

		InstanceBufferResource* GetResource() { return m_Resource; }
		InstanceBufferView* GetView() { return m_View; }

		uint32 GetCount() { return m_Count; }
		uint32 GetCapcity() { return m_Capacity; }
		void* GetData() { return m_Data; }
		
		void PushBack(const void* data, uint32 count = 1);
		void PopBack() { m_Count--; }

		void Clear() { m_Count = 0; }
		void Apply();
		void* At(uint32 index) { return (void*)((uint64)m_Data + (index * m_Stride)); }
		template<typename T>
		T& Get(uint32 index) { return *(T*)At(index); }
		bool Empty() { return GetCount() == 0; }

		static Ref<InstanceBuffer> Create(uint32 capacity, uint32 stride);

	protected:
		InstanceBufferResource* m_Resource;
		InstanceBufferView* m_View;

		uint32 m_Count;
		uint32 m_Capacity;
		uint32 m_Stride;
		void* m_Data;
	};

}
