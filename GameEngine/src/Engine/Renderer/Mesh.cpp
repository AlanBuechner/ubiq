#include "pch.h"
#include "Mesh.h"

namespace Engine
{

	Mesh::Mesh(uint32 vertexStride) :
		m_VertexStride(vertexStride)
	{}

	void Mesh::SetVertices(const void* vertices, uint32 count)
	{
		if (m_VertexBuffer)
		{
			if (count != m_VertexBuffer->GetResource()->GetCount())
				m_VertexBuffer->Resize(count);
			m_VertexBuffer->SetData(vertices);
		}
		else
			m_VertexBuffer = VertexBuffer::Create(vertices, count, m_VertexStride);
	}

	void Mesh::SetIndices(const uint32* data, uint32 count)
	{
		if (m_IndexBuffer)
		{
			if (count != m_IndexBuffer->GetResource()->GetCount())
				m_IndexBuffer->Resize(count);
			m_IndexBuffer->SetData(data);
		}
		else
			m_IndexBuffer = IndexBuffer::Create(data, count);
	}

	Ref<Mesh> Mesh::Create(uint32 vertexStride)
	{
		return CreateRef<Mesh>(vertexStride);
	}

}
