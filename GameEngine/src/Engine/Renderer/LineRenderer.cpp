#include "pch.h"
#include "LineRenderer.h"
#include "Renderer.h"
#include "Resources/Buffer.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Mesh.h"
#include "CommandList.h"

#include "EngineResource.h"

namespace Engine
{

	struct LineRendererData
	{
		static const uint32 LineVertexCount = 2;
		static const uint32 MaxLines = 10000;
		static const uint32 MaxVertices = MaxLines * LineVertexCount;
		static const uint32 MaxIndices = MaxLines * 2;

		Ref<Shader> LineShader;

		std::vector<LineVertex> Vertices;
		std::vector<uint32> Indices;

		Ref<InstanceBuffer> Instances;
		Ref<Mesh> m_Mesh;
		Ref<ConstantBuffer> Camera;
	};

	static LineRendererData s_LineData;

	void LineRenderer::Init()
	{
		s_LineData.LineShader = Shader::CreateFromEmbeded(LINE, "LineShader.hlsl");

		s_LineData.Vertices.reserve(s_LineData.MaxVertices);
		s_LineData.Indices.reserve(s_LineData.MaxIndices);

		s_LineData.m_Mesh = Mesh::Create(sizeof(LineVertex));

		s_LineData.Instances = InstanceBuffer::Create(sizeof(Math::Mat4), 1);
		Math::Mat4 transform = Math::Mat4(1);
		s_LineData.Instances->SetData(0, 1, &transform);
		s_LineData.Instances->Apply();

		s_LineData.Camera = ConstantBuffer::Create(sizeof(Math::Mat4));
	}

	void LineRenderer::Destroy()
	{
		s_LineData.Camera.reset();
		s_LineData.Instances.reset();
		s_LineData.m_Mesh.reset();
		s_LineData.LineShader.reset();
	}

	void LineRenderer::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		Math::Mat4 viewProj = camera.GetProjectionMatrix() * Math::Inverse(transform);
		s_LineData.Camera->SetData(&viewProj);
	}

	void LineRenderer::BeginScene(Ref<EditorCamera> camera)
	{
		Math::Mat4 viewProj = camera->GetViewProjection();
		s_LineData.Camera->SetData(&viewProj);
	}

	void LineRenderer::EndScene()
	{
		s_LineData.m_Mesh->SetVertices(s_LineData.Vertices.data(), (uint32)s_LineData.Vertices.size());
		s_LineData.m_Mesh->SetIndices(s_LineData.Indices.data(), (uint32)s_LineData.Indices.size());
	}

	void LineRenderer::Build(Ref<CommandList> commandList)
	{
		if (!s_LineData.Vertices.empty())
		{
			commandList->SetShader(s_LineData.LineShader->GetPass("main"));
			commandList->SetConstantBuffer(0, s_LineData.Camera);
			commandList->DrawMesh(s_LineData.m_Mesh, s_LineData.Instances);
		}

		s_LineData.Vertices.clear();
		s_LineData.Indices.clear();
	}

	void LineRenderer::DrawLine(Math::Vector3 p1, Math::Vector3 p2, const Math::Vector4 color, const Math::Mat4& transform)
	{
		LineVertex vert;
		vert.Color = color;

		uint32 baseVertexIndex = (uint32)s_LineData.Vertices.size();
		s_LineData.Indices.push_back(baseVertexIndex);
		s_LineData.Indices.push_back(baseVertexIndex+1);

		vert.Position = transform * Math::Vector4(p1, 1.0f);
		s_LineData.Vertices.push_back(vert); 
		vert.Position = transform * Math::Vector4(p2, 1.0f);
		s_LineData.Vertices.push_back(vert);
	}

	void LineRenderer::DrawLineMesh(LineMesh& mesh, const Math::Mat4& transform)
	{

		uint32 baseVertexIndex = (uint32)s_LineData.Vertices.size();
		for (uint32 i = 0; i < mesh.m_Vertices.size(); i++)
		{
			LineVertex vert = mesh.m_Vertices[i];
			vert.Position = transform * Math::Vector4(vert.Position, 1.0f);
			s_LineData.Vertices.push_back(vert);
		}

		for (uint32 i = 0; i < mesh.m_Indices.size(); i++)
			s_LineData.Indices.push_back(baseVertexIndex + mesh.m_Indices[i]);

	}

}
