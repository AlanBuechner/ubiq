#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
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
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}

	void Renderer::BeginScene(Camera& camera)
	{

	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const glm::mat4& transform)
	{

	}
}