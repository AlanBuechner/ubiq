#include "DynamicBuffer.h"
#include <memory>

namespace Utils
{

	DynamicBuffer::DynamicBuffer(uint32 stride) :
		m_Stride(stride)
	{}

	void DynamicBuffer::SetStride(uint32 stride)
	{
		if(m_Data != nullptr)
			delete m_Data;
		m_Count = 0;
		m_Capacity = 0;
		m_Stride = stride;
	}

	void DynamicBuffer::Resize(uint32 count)
	{
		if (count <= m_Capacity)
			m_Count = count;
		else
		{
			AllocateAndCopy(count);
			m_Count = count;
		}
	}

	void DynamicBuffer::Reserve(uint32 cap)
	{
		if (cap > m_Capacity)
			AllocateAndCopy(cap);
	}

	void DynamicBuffer::AllocateAndCopy(uint32 cap)
	{
		void* data = new byte[cap * m_Stride];
		m_Capacity = cap;

		memcpy(data, m_Data, m_Count * m_Stride);

		delete m_Data;
		m_Data = data;
		m_Capacity = cap;
	}

}

