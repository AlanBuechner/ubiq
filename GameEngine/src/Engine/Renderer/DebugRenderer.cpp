#include "pch.h"
#include "DebugRenderer.h"
#include "Renderer.h"
#include "Abstractions/Resources/Buffer.h"
#include "Abstractions/Shader.h"
#include "Abstractions/CommandList.h"
#include "Abstractions/Resources/InstanceBuffer.h"
#include "Abstractions/CommandList.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Mesh.h"

#include "EngineResource.h"

#include "Utils/Common.h"

namespace Engine
{
	struct Pass
	{
		Ref<ShaderPass> m_Pass;
		Ref<Mesh> m_Mesh;

		std::vector<DebugVertex> Vertices;
		std::vector<uint32> Indices;
	};

	struct DebugRendererData
	{

		Ref<Shader> DebugShader;
		Ref<ConstantBuffer> Camera;

		Pass DepthTestLinesPass;
		Pass DepthTestMeshPass;
		Pass LinesPass;
		Pass MeshPass;

		static const uint32 NumPasses = 4;
		Pass* Passes[NumPasses] = {
			&DepthTestLinesPass,
			&LinesPass,
			&DepthTestMeshPass,
			&MeshPass,
		};
	};

	static DebugRendererData s_DebugData;

	void DebugRenderer::Init()
	{
		s_DebugData.Camera = ConstantBuffer::Create(sizeof(Math::Mat4));

		s_DebugData.DebugShader = Shader::CreateFromEmbeded(LINE, "LineShader.hlsl");

		// init depth test line pass
		s_DebugData.DepthTestLinesPass.m_Pass = s_DebugData.DebugShader->GetPass("DepthTestLinePass");
		s_DebugData.DepthTestLinesPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init non-depth test line pass
		s_DebugData.LinesPass.m_Pass = s_DebugData.DebugShader->GetPass("LinePass");
		s_DebugData.LinesPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init depth test mesh pass
		s_DebugData.DepthTestMeshPass.m_Pass = s_DebugData.DebugShader->GetPass("DepthTestMeshPass");
		s_DebugData.DepthTestMeshPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init non-depth test mesh pass
		s_DebugData.MeshPass.m_Pass = s_DebugData.DebugShader->GetPass("MeshPass");
		s_DebugData.MeshPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));
	}

	void DebugRenderer::Destroy()
	{
		for (uint32 i = 0; i < s_DebugData.NumPasses; i++)
		{
			Pass* pass = s_DebugData.Passes[i];
			pass->m_Pass.reset();
			pass->m_Mesh.reset();
		}

		s_DebugData.DebugShader.reset();
		s_DebugData.Camera.reset();
	}

	void DebugRenderer::EndScene()
	{
		for (uint32 i = 0; i < s_DebugData.NumPasses; i++) 
		{
			Pass* pass = s_DebugData.Passes[i];
			std::vector<DebugVertex>& verts = pass->Vertices;
			std::vector<uint32>& indices = pass->Indices;
			if (!verts.empty())
			{
				pass->m_Mesh->SetVertices(verts.data(), (uint32)verts.size());
				pass->m_Mesh->SetIndices(indices.data(), (uint32)indices.size());
			}
		}
	}

	void DebugRenderer::SetCamera(const Camera& camera)
	{
		Math::Mat4 viewProj = camera.GetViewProjectionMatrix();
		s_DebugData.Camera->SetData(&viewProj);
	}


	void DebugRenderer::Build(Ref<CommandList> commandList)
	{

		for (uint32 i = 0; i < s_DebugData.NumPasses; i++)
		{
			Pass* pass = s_DebugData.Passes[i];
			std::vector<DebugVertex>& verts = pass->Vertices;
			std::vector<uint32>& indices = pass->Indices;

			if (!verts.empty())
			{
				commandList->SetShader(pass->m_Pass);
				commandList->SetConstantBuffer(0, s_DebugData.Camera);
				commandList->DrawMesh(pass->m_Mesh);
			}

			verts.clear();
			indices.clear();
		}
	}

	void DebugRenderer::DrawLine(Math::Vector3 p1, Math::Vector3 p2, Math::Vector4 color, const Math::Mat4& transform, bool depthTest)
	{
		DebugVertex vert;
		vert.Color = color;

		Pass& pass = depthTest ? s_DebugData.DepthTestLinesPass : s_DebugData.LinesPass;

		uint32 baseVertexIndex = (uint32)pass.Vertices.size();
		pass.Indices.push_back(baseVertexIndex);
		pass.Indices.push_back(baseVertexIndex + 1);

		vert.Position = transform * Math::Vector4(p1, 1.0f);
		pass.Vertices.push_back(vert); 
		vert.Position = transform * Math::Vector4(p2, 1.0f);
		pass.Vertices.push_back(vert);
	}

	void DebugRenderer::DrawLineMesh(DebugMesh& mesh, const Math::Mat4& transform, bool depthTest)
	{
		Pass& pass = depthTest ? s_DebugData.DepthTestLinesPass : s_DebugData.LinesPass;
		DrawMesh(mesh, transform, pass);
	}

	void DebugRenderer::DrawWireBox(const Math::Mat4& transform, Math::Vector4 color, bool depthTest)
	{
		Engine::DebugMesh mesh;
		mesh.m_Vertices = {
			{{-1,  1, -1}, color}, // 0
			{{ 1,  1, -1}, color}, // 1
			{{ 1, -1, -1}, color}, // 2
			{{-1, -1, -1}, color}, // 3
			{{-1,  1,  1}, color}, // 4
			{{ 1,  1,  1}, color}, // 5
			{{ 1, -1,  1}, color}, // 6
			{{-1, -1,  1}, color}, // 7
		};

		mesh.m_Indices = {
			0,4, 1,5, 2,6, 3,7,
			0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4
		};

		DrawLineMesh(mesh, transform, depthTest);
	}

	void DebugRenderer::DrawWireBox(Math::Vector3 center, Math::Vector3 halfExtent, Math::Vector4 color, const Math::Mat4& transform, bool depthTest)
	{
		DrawWireBox(transform * Math::Translate(center) * Math::Scale(halfExtent), color, depthTest);
	}

	void DebugRenderer::DrawMesh(DebugMesh& mesh, const Math::Mat4& transform, bool depthTest)
	{
		Pass& pass = depthTest ? s_DebugData.DepthTestMeshPass : s_DebugData.MeshPass;
		DrawMesh(mesh, transform, pass);
	}

	void DebugRenderer::DrawBox(const Math::Mat4& transform, Math::Vector4 color, bool depthTest /*= true*/)
	{
		DebugMesh mesh;
		mesh.m_Vertices = {
			{{-1,  1, -1}, color}, // 0
			{{ 1,  1, -1}, color}, // 1
			{{ 1, -1, -1}, color}, // 2
			{{-1, -1, -1}, color}, // 3
			{{-1,  1,  1}, color}, // 4
			{{ 1,  1,  1}, color}, // 5
			{{ 1, -1,  1}, color}, // 6
			{{-1, -1,  1}, color}, // 7
		};

		mesh.m_Indices = {
			0,7,4, 0,3,7,
			0,4,5, 0,5,1,
			0,1,3, 3,1,2,

			6,2,1, 6,1,5,
			6,3,2, 6,7,3,
			6,5,7, 5,4,7,
		};

		DrawMesh(mesh, transform, depthTest);
	}

	void DebugRenderer::DrawBox(Math::Vector3 center, Math::Vector3 halfExtent, Math::Vector4 color, const Math::Mat4& transform /*= Math::Mat4(1.0f)*/, bool depthTest /*= true*/)
	{
		DrawBox(transform * Math::Translate(center) * Math::Scale(halfExtent), color, depthTest);
	}


	void DebugRenderer::DrawMesh(const DebugMesh& mesh, const Math::Mat4& transform, Pass& pass)
	{
		uint32 baseVertexIndex = (uint32)pass.Vertices.size();
		for (uint32 i = 0; i < mesh.m_Vertices.size(); i++)
		{
			DebugVertex vert = mesh.m_Vertices[i];
			vert.Position = transform * Math::Vector4(vert.Position, 1.0f);
			pass.Vertices.push_back(vert);
		}

		for (uint32 i = 0; i < mesh.m_Indices.size(); i++)
			pass.Indices.push_back(baseVertexIndex + mesh.m_Indices[i]);
	}

}
