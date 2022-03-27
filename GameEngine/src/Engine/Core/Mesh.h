#pragma once
#include "core.h"
#include <Engine/Math/Math.h>

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
	class TMesh
	{
	public:
		using Vertex = V;

		std::vector<V> vertices;
		std::vector<uint32_t> indices;
	};

	using Mesh = TMesh<RendererVertex>;
}
