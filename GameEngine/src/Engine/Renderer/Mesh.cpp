#include "pch.h"
#include "Mesh.h"

#include "Utils/Performance.h"

namespace Engine
{

	Mesh::Mesh(uint32 vertexStride) :
		m_VertexBuffers({ VertexBuffer::Create(1, vertexStride) })
	{}

	Mesh::Mesh(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib) :
		m_VertexBuffers({ vb }), m_IndexBuffer(ib)
	{}

	Mesh::Mesh(Utils::Vector<uint32> vertexStrides)
	{
		m_VertexBuffers.Resize(vertexStrides.Count());
		for (uint32 i = 0; i < vertexStrides.Count(); i++)
			m_VertexBuffers.Push(VertexBuffer::Create(1, vertexStrides[i]));
	}

	Mesh::Mesh(Utils::Vector<Ref<VertexBuffer>> vbs, Ref<IndexBuffer> ib) :
		m_VertexBuffers(vbs), m_IndexBuffer(ib)
	{}

	void Mesh::AddStream(uint32 stream, uint32 stride)
	{
		if (m_VertexBuffers.Count() >= stream)
			m_VertexBuffers.Resize(stream + 1);
		m_VertexBuffers[stream] = VertexBuffer::Create(1, stride);
	}

	void Mesh::SetVertices(const void* vertices, uint32 count, uint32 stream)
	{
		CORE_ASSERT(count != 0, "Vertex count cant be 0");

		if (count != m_VertexBuffers[stream]->GetResource()->GetCount())
			m_VertexBuffers[stream]->Resize(count);
		m_VertexBuffers[stream]->SetData(vertices, count);
	}

	void Mesh::SetIndices(const uint32* data, uint32 count)
	{
		if (count == 0)
		{
			m_IndexBuffer = nullptr;
			return;
		}

		if (m_IndexBuffer)
		{
			if (count != m_IndexBuffer->GetResource()->GetCount())
				m_IndexBuffer->Resize(count);
			m_IndexBuffer->SetData(data, count);
		}
		else
			m_IndexBuffer = IndexBuffer::Create(data, count);
	}

	void Mesh::SetVertexBuffer(Ref<VertexBuffer> vb, uint32 stream)
	{
		if (m_VertexBuffers.Count() >= stream)
			m_VertexBuffers.Resize(stream + 1);

		m_VertexBuffers[stream] = vb;
	}

	void Mesh::SetIndexBuffer(Ref<IndexBuffer> ib)
	{
		m_IndexBuffer = ib;
	}

	Ref<Mesh> Mesh::Create(uint32 vertexStride)
	{
		return CreateRef<Mesh>(vertexStride);
	}

	Ref<Mesh> Mesh::Create(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib)
	{
		return CreateRef<Mesh>(vb, ib);
	}

	Ref<Mesh> Mesh::Create(Utils::Vector<Ref<VertexBuffer>> vbs, Ref<IndexBuffer> ib)
	{
		return CreateRef<Mesh>(vbs, ib);
	}

}
