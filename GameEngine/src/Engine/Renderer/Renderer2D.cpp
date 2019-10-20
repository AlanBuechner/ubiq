#include "pch.h"
#include "Renderer2D.h"

#include "Engine/Core/core.h"
#include "Renderer.h"
#include "Shader.h"
#include "OrthographicCamera.h"

namespace Engine
{

	struct Renderer2DStroage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
	};

	static Renderer2DStroage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStroage();

		s_Data->QuadVertexArray = Engine::VertexArray::Create();

		float vertices[4 * 5] =
		{
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> m_VertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"}
		};

		m_VertexBuffer->SetLayout(layout);

		s_Data->QuadVertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 2, 1, 0,  0, 3, 2 };

		Ref<IndexBuffer> m_IndexBuffer = Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t));
		s_Data->QuadVertexArray->SetIndexBuffer(m_IndexBuffer);

		Engine::Ref<Engine::Shader::ShaderSorce> src = std::make_shared<Engine::Shader::ShaderSorce>();

		*src << Shader::LoadShader("Assets/Shaders/FlatColorShader.glsl");

		s_Data->FlatColorShader = Shader::Create("FlatColorShader", src);
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->FlatColorShader->Bind();
		s_Data->FlatColorShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		s_Data->FlatColorShader->Bind();
		s_Data->FlatColorShader->UploadUniformMat4("u_Transform", glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size, 0.0f }));
		s_Data->FlatColorShader->UploadUniformFloat4("u_Color", color);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}