#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"

namespace Engine
{

	// Constant Buffer Resource ---------------------------------------------------------- //

	class ConstantBufferResource : public GPUResource
	{
	public:
		virtual ~ConstantBufferResource() = 0;

		uint32 GetSize() { return m_Size; }

		virtual bool SupportState(ResourceState state) override;

		virtual void SetData(const void* data) = 0;

		static ConstantBufferResource* Create(uint32 size);

	protected:
		uint32 m_Size = 0;
	};
	
	// Descriptor Handles ---------------------------------------------------------- //

	class ConstantBufferCBVDescriptorHandle : public Descriptor
	{
	public:
		virtual uint64 GetGPUHandlePointer() const = 0;
		virtual uint32 GetIndex() const = 0;
		virtual void ReBind(ConstantBufferResource* resource) = 0;

		static ConstantBufferCBVDescriptorHandle* Create(ConstantBufferResource* resource);
	};

	// Constant Buffer Objects ---------------------------------------------------------- //

	class ConstantBuffer
	{
	public:
		ConstantBuffer(uint32 size);
		virtual ~ConstantBuffer();
		
		void SetData(const void* data) { m_Resource->SetData(data); }

		ConstantBufferResource* GetResource() { return m_Resource; }
		ConstantBufferCBVDescriptorHandle* GetCBVDescriptor() { return m_CBVDesciptor; }

		static Ref<ConstantBuffer> Create(uint32 size);

	protected:
		ConstantBufferResource* m_Resource;
		ConstantBufferCBVDescriptorHandle* m_CBVDesciptor;
	};


}
