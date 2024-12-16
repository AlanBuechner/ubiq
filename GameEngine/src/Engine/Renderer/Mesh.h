#pragma once
#include "Abstractions/Resources/Buffer.h"

namespace Engine
{
	class Mesh
	{
	public:
		Mesh(uint32 vertexStride);
		Mesh(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib);

		inline Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }
	
		void SetVertices(const void* vertices, uint32 count);
		void SetIndices(const uint32* data, uint32 count);

		void SetVertexBuffer(Ref<VertexBuffer> vb);
		void SetIndexBuffer(Ref<IndexBuffer> ib);

		static Ref<Mesh> Create(uint32 vertexStride);
		static Ref<Mesh> Create(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib);

	private:
		uint32 m_VertexStride;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
