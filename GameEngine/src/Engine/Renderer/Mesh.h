#pragma once
#include "Abstractions/Resources/Buffer.h"

namespace Engine
{
	class Mesh
	{
	public:
		Mesh(uint32 vertexStride);
		Mesh(Utils::Vector<uint32> vertexStrides);
		Mesh(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib);
		Mesh(Utils::Vector<Ref<VertexBuffer>> vbs, Ref<IndexBuffer> ib);

		inline Utils::Vector<Ref<VertexBuffer>>& GetVertexBuffers() { return m_VertexBuffers; }
		inline Ref<VertexBuffer> GetVertexBuffer(uint32 i) { return m_VertexBuffers[i]; }
		inline Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }
	
		void AddStream(uint32 stream, uint32 stride);

		void SetVertices(const void* vertices, uint32 count, uint32 stream = 0);
		void SetIndices(const uint32* data, uint32 count);

		void SetVertexBuffer(Ref<VertexBuffer> vb, uint32 stream = 0);
		void SetIndexBuffer(Ref<IndexBuffer> ib);

		static Ref<Mesh> Create(uint32 vertexStride);
		static Ref<Mesh> Create(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib);
		static Ref<Mesh> Create(Utils::Vector<Ref<VertexBuffer>> vbs, Ref<IndexBuffer> ib);

	private:

		Utils::Vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
