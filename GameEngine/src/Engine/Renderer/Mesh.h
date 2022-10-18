#pragma once
#include "Buffer.h"
#include "Engine/AssetManager/AssetManager.h"

namespace Engine
{
	class Mesh : public Asset
	{
	public:
		Mesh(uint32 vertexStride);

		void SetVertices(const void* vertices, uint32 count);
		void SetIndices(const uint32* data, uint32 count);

		inline Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

		static Ref<Mesh> Create(BufferLayout layout);
		static Ref<Mesh> Create(uint32 vertexStride);
		static Ref<Mesh> Create(const fs::path& path);

		static bool ValidExtention(const fs::path& ext);

	private:
		uint32 m_VertexStride;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
