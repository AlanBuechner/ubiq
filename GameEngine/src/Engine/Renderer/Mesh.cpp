#include "pch.h"
#include "Mesh.h"

#include "Engine/Core/MeshLoader.h"

namespace Engine
{

	Mesh::Mesh(uint32 vertexStride) :
		m_VertexStride(vertexStride)
	{}

	void Mesh::SetVertices(const void* vertices, uint32 count)
	{
		if (m_VertexBuffer)
			m_VertexBuffer->SetData(vertices, count);
		else
			m_VertexBuffer = VertexBuffer::Create(vertices, count, m_VertexStride);
	}

	void Mesh::SetIndices(const uint32* data, uint32 count)
	{
		if (m_IndexBuffer)
			m_IndexBuffer->SetData(data, count);
		else
			m_IndexBuffer = IndexBuffer::Create(data, count);
	}

	Ref<Mesh> Mesh::Create(BufferLayout layout)
	{
		return CreateRef<Mesh>(layout.GetStride());
	}

	Ref<Mesh> Mesh::Create(uint32 vertexStride)
	{
		return CreateRef<Mesh>(vertexStride);
	}

}
