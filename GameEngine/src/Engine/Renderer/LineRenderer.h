#pragma once
#include "glm/glm.hpp"

namespace Engine
{
	class Camera;
	class EditorCamera;
}

namespace Engine
{
	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

	};

	class LineRenderer
	{
	public:
		static void Init();
		static void ShutDown();

	public:

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();

	private:
		static void BeginBatch();

	public:

		static void DrawLine(glm::vec3 p1, glm::vec3 p2, const glm::vec4 color, const glm::mat4& transform = glm::mat4(1.0f));

	};
}