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

		static void OnWindowResize(uint32 width, uint32 height);

		static void BeginScene(const Camera& camera, const Math::Mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, const Ref<Material>& mat, const Math::Mat4& transform = Math::Mat4(1.0f));
		static void SubmitPointLight(const PointLight& light);
		static void SetAmbientLight(Math::Vector3 l) { m_SceneData->ambientLight = l; }

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData
		{
			Math::Mat4 ViewMatrix;
			Math::Mat4 ProjectionMatrix;
			std::vector<PointLight> PointLights;
			Math::Vector3 ambientLight;
		};

		static SceneData* m_SceneData;
	};
}
