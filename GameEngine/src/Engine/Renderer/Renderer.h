#pragma once
#include "Engine\Core\core.h"
#include "RenderCommand.h"
#include "Material.h"

#include "EditorCamera.h"
#include "Light.h"

#define MAX_LIGHTS 12

namespace Engine
{
	class Camera;
	class Shader;
}

namespace Engine
{

	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, const Ref<Material>& mat, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitPointLight(const PointLight& light);
		static void SetAmbientLight(glm::vec3 l) { m_SceneData->ambientLight = l; }

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData
		{
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			std::vector<PointLight> PointLights;
			glm::vec3 ambientLight;
		};

		static SceneData* m_SceneData;
	};
}
