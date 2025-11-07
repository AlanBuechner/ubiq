#pragma once
#include "Common.h"
#include "Types.h"

namespace Utils
{
	class DynamicBuffer
	{
	public:

		DynamicBuffer() = default;
		DynamicBuffer(uint32 stride);

		void SetStride(uint32 stride);
		uint32 Stride() { return m_Stride; }
		uint32 Count() { return m_Count; }
		uint32 Capacity() { return m_Capacity; }

		template<typename T>
		T* Push(const T& data)
		{
			if (m_Count + 1 > m_Capacity)
				ReserveMore(std::max(m_Count, 1u)); // double count

			T* d = GetData<T>(m_Count);
			(*d) = data;
			m_Count++;
			return d;
		}

		template<typename T>
		T* GetData(uint32 i = 0) { return &((T*)m_Data)[i]; }
		void* GetData() { return m_Data; }

		void Resize(uint32 count);
		void Reserve(uint32 cap);
		void ReserveMore(uint32 cap) { Reserve(m_Count + cap); }

		void Clear() { m_Count = 0; }


	private:

		void AllocateAndCopy(uint32 cap);

		uint32 m_Stride = 0;
		uint32 m_Count = 0;
		uint32 m_Capacity = 0;

		void* m_Data = nullptr;
	};
}
