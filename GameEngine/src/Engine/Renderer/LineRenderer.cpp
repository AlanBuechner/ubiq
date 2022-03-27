#include "pch.h"
#include "RenderCommand.h"
#include "LineRenderer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"

namespace Engine
{

	struct LineRendererData
	{
		static const uint32_t LineVertexCount = 2;
		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxVertices = MaxLines * LineVertexCount;
		static const uint32_t MaxIndices = MaxLines * 2;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<IndexBuffer> LineIndexBuffer;
		Ref<ShaderLibrary> Library;

		uint32_t VertexCount = 0;
		uint32_t IndexCount = 0;

		uint32_t* LineIndexBufferBase = nullptr;
		uint32_t* LineIndexBufferPtr = nullptr;

		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
	};

	static LineRendererData s_Data;

	void LineRenderer::Init()
	{
		s_Data.LineVertexArray = Engine::VertexArray::Create();

		s_Data.LineVertexBuffer = Engine::VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float4, "a_Color"}
		};

		s_Data.LineVertexBuffer->SetLayout(layout);
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);

		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

		s_Data.LineIndexBufferBase = new uint32_t[s_Data.MaxIndices];

		s_Data.LineIndexBuffer = Engine::IndexBuffer::Create(s_Data.MaxIndices);
		s_Data.LineVertexArray->SetIndexBuffer(s_Data.LineIndexBuffer);

		s_Data.Library = std::make_shared<ShaderLibrary>();
		Ref<Shader> TextureShader = s_Data.Library->Load("LineShader", "Assets/Shaders/LineShader.glsl");
	}

	void LineRenderer::ShutDown()
	{
	}

	void LineRenderer::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		Math::Mat4 viewProj = camera.GetProjectionMatrix() * glm::inverse(transform);

		Ref<Shader> LineShader = s_Data.Library->Get("LineShader");
		LineShader->Bind();
		LineShader->UploadUniformMat4("u_ViewProjection", viewProj);
		s_Data.LineVertexArray->Bind();

		BeginBatch();
	}

	void LineRenderer::BeginScene(const EditorCamera& camera)
	{
		Math::Mat4 viewProj = camera.GetViewProjection();

		Ref<Shader> LineShader = s_Data.Library->Get("LineShader");
		LineShader->Bind();
		LineShader->UploadUniformMat4("u_ViewProjection", viewProj);
		s_Data.LineVertexArray->Bind();

		BeginBatch();
	}

	void LineRenderer::EndScene()
	{
		s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, s_Data.VertexCount * sizeof(LineVertex));

		s_Data.LineIndexBuffer->SetData(s_Data.LineIndexBufferBase, s_Data.IndexCount);

		Flush();
	}

	void LineRenderer::Flush()
	{
		if (s_Data.IndexCount != 0)
			RenderCommand::DrawLineIndexed(s_Data.LineVertexArray, s_Data.IndexCount);
	}

	void LineRenderer::BeginBatch()
	{
		s_Data.IndexCount = 0;
		s_Data.VertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
		s_Data.LineIndexBufferPtr = s_Data.LineIndexBufferBase;
	}

	void LineRenderer::DrawLine(Math::Vector3 p1, Math::Vector3 p2, const Math::Vector4 color, const Math::Mat4& transform)
	{
		if (s_Data.IndexCount + 2 > s_Data.MaxIndices || s_Data.VertexCount + 2 > s_Data.MaxVertices)
		{
			EndScene();
			BeginBatch();
		}

		s_Data.LineVertexBufferPtr->Position = transform * Math::Vector4(p1, 1.0f);
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		*s_Data.LineIndexBufferPtr = s_Data.VertexCount + 0;
		s_Data.LineIndexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = transform * Math::Vector4(p2, 1.0f);
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		*s_Data.LineIndexBufferPtr = s_Data.VertexCount + 1;
		s_Data.LineIndexBufferPtr++;

		s_Data.IndexCount += 2;
		s_Data.VertexCount += 2;

	}

	void LineRenderer::DrawLineMesh(LineMesh& mesh, const Math::Mat4& transform)
	{
		if (s_Data.IndexCount + mesh.m_Indices.size() > s_Data.MaxIndices || s_Data.VertexCount + mesh.m_Vertices.size() > s_Data.MaxVertices)
		{
			EndScene();
			BeginBatch();
		}

		for (uint32_t i = 0; i < mesh.m_Vertices.size(); i++)
		{
			s_Data.LineVertexBufferPtr->Position = transform * Math::Vector4(mesh.m_Vertices[i].Position, 1.0f);
			s_Data.LineVertexBufferPtr->Color = mesh.m_Vertices[i].Color;
			s_Data.LineVertexBufferPtr++;
		}

		for (uint32_t i = 0; i < mesh.m_Indices.size(); i++)
		{
			*s_Data.LineIndexBufferPtr = s_Data.VertexCount + mesh.m_Indices[i];
			s_Data.LineIndexBufferPtr++;
		}


		s_Data.IndexCount += (uint32_t)mesh.m_Indices.size();
		s_Data.VertexCount += (uint32_t)mesh.m_Vertices.size();
	}

}
