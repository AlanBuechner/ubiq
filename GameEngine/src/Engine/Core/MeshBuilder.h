#pragma once
#include "Core.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Math/Math.h"

#include <vector>

namespace Engine
{
	struct RendererVertex
	{
		Math::Vector3 position;
		Math::Vector2 uv;
		Math::Vector3 normal;
		Math::Vector3 tangent;
	};

	template<class V>
	class TMeshBuilder
	{
	public:
		using Vertex = V;

		TMeshBuilder() = default;
		TMeshBuilder(const TMeshBuilder<V>& other);

		std::vector<V> vertices;
		std::vector<uint32> indices;

		Ref<Mesh> mesh = Mesh::Create(sizeof(V));

		void Apply()
		{
			mesh->SetVertices(vertices.data(), (uint32)vertices.size());
			mesh->SetIndices(indices.data(), (uint32)indices.size());
		}
	};

	template<class V>
	TMeshBuilder<V>::TMeshBuilder(const TMeshBuilder<V>& other)
	{
		vertices = other.vertices;
		indices = other.indices;
		Apply();
	}

	typedef TMeshBuilder<RendererVertex> MeshBuilder;

	MeshBuilder CreateUVSphere(uint32 uSeg, uint32 vSeg);
}
