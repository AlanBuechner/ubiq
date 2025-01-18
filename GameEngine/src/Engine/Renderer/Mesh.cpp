#include "pch.h"
#include "Mesh.h"

#include "Utils/Performance.h"

namespace Engine
{

	Mesh::Mesh(uint32 vertexStride) :
		m_VertexStride(vertexStride)
	{}

	Mesh::Mesh(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib) :
		m_VertexBuffer(vb), m_IndexBuffer(ib), m_VertexStride(vb->GetStride())
	{}

	void Mesh::SetVertices(const void* vertices, uint32 count)
	{
		if (count == 0)
		{
			m_VertexBuffer = nullptr;
			return;
		}

		if (m_VertexBuffer)
		{
			if (count != m_VertexBuffer->GetResource()->GetCount())
				m_VertexBuffer->Resize(count);
			m_VertexBuffer->SetData(vertices, count);
		}
		else
			m_VertexBuffer = VertexBuffer::Create(vertices, count, m_VertexStride);
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

	void Mesh::SetVertexBuffer(Ref<VertexBuffer> vb)
	{
		m_VertexStride = vb->GetStride();
		m_VertexBuffer = vb;
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

}
