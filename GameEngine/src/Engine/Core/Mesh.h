#pragma once
#include "core.h"
#include <glm\glm.hpp>

#include <vector>

namespace Engine
{
	struct RendererVertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
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
