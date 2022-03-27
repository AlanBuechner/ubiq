#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "LineRenderer.h"

namespace Engine
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		ShaderLibrary::Init();
		RenderCommand::Init();
		LineRenderer::Init();
		Renderer2D::Init();

		m_SceneData->PointLights.reserve(MAX_LIGHTS);
	}

	void Renderer::OnWindowResize(uint32 width, uint32 height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		m_SceneData->ViewMatrix = camera.GetViewMatrix();
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
	}

	void Renderer::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		m_SceneData->ViewMatrix = transform;
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
	}

	void Renderer::EndScene()
	{
		m_SceneData->PointLights.clear();
		m_SceneData->PointLights.reserve(MAX_LIGHTS);
	}

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Material>& mat, const Math::Mat4& transform)
	{
		mat->shader->Bind();

		mat->shader->UploadUniformMat4("u_View", m_SceneData->ViewMatrix);
		mat->shader->UploadUniformMat4("u_Projection", m_SceneData->ProjectionMatrix);
		mat->shader->UploadUniformMat4("u_Transform", transform);

		mat->shader->UploadUniformInt("u_Diffuse", 1);
		mat->shader->UploadUniformInt("u_Specular", 2);
		mat->shader->UploadUniformInt("u_NormalMap", 3);

		if (mat->diffuse)
			mat->diffuse->Bind(1);

		if (mat->speculur)
			mat->speculur->Bind(2);

		if (mat->normal)
			mat->normal->Bind(3);

		mat->shader->UploadUniformInt("numLights", (uint32)m_SceneData->PointLights.size());
		for (uint32 i = 0; i < m_SceneData->PointLights.size(); i++)
			mat->shader->UploadPointLight("pointLights", i, m_SceneData->PointLights[i]);
		mat->shader->UploadUniformFloat3("ambientLight", m_SceneData->ambientLight);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);

	}

	void Renderer::SubmitPointLight(const PointLight& light)
	{
		if (m_SceneData->PointLights.size() >= MAX_LIGHTS)
			return;
		m_SceneData->PointLights.push_back(light);
	}

}
