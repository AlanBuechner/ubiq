#include "pch.h"
#include "DebugRenderer.h"
#include "Renderer.h"
#include "Abstractions/Resources/Buffer.h"
#include "Shaders/Shader.h"
#include "Abstractions/CommandList.h"
#include "Camera.h"
#include "Mesh.h"

#include "EngineResource.h"

#include "Utils/Common.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Engine
{
	struct Pass
	{
		Ref<GraphicsShaderPass> m_Pass;
		Ref<Mesh> m_Mesh;

		Utils::Vector<DebugVertex> Vertices;
		Utils::Vector<uint32> Indices;
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

		s_DebugData.DebugShader = Application::Get().GetAssetManager().GetEmbededAsset<Shader>(DEBUGSHADER);

		// init depth test line pass
		s_DebugData.DepthTestLinesPass.m_Pass = s_DebugData.DebugShader->GetGraphicsPass("DepthTestLinePass");
		s_DebugData.DepthTestLinesPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init non-depth test line pass
		s_DebugData.LinesPass.m_Pass = s_DebugData.DebugShader->GetGraphicsPass("LinePass");
		s_DebugData.LinesPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init depth test mesh pass
		s_DebugData.DepthTestMeshPass.m_Pass = s_DebugData.DebugShader->GetGraphicsPass("DepthTestMeshPass");
		s_DebugData.DepthTestMeshPass.m_Mesh = Mesh::Create(sizeof(DebugVertex));

		// init non-depth test mesh pass
		s_DebugData.MeshPass.m_Pass = s_DebugData.DebugShader->GetGraphicsPass("MeshPass");
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
		CREATE_PROFILE_FUNCTIONI();
		for (uint32 i = 0; i < s_DebugData.NumPasses; i++) 
		{
			Pass* pass = s_DebugData.Passes[i];
			Utils::Vector<DebugVertex>& verts = pass->Vertices;
			Utils::Vector<uint32>& indices = pass->Indices;
			if (!verts.Empty())
			{
				pass->m_Mesh->SetVertices(verts.Data(), (uint32)verts.Count());
				pass->m_Mesh->SetIndices(indices.Data(), (uint32)indices.Count());
			}
		}
	}

	void DebugRenderer::SetCamera(const Camera& camera)
	{
		Math::Mat4 viewProj = camera.GetViewProjectionMatrix();
		s_DebugData.Camera->SetData(&viewProj);
	}


	void DebugRenderer::Build(Ref<CPUCommandList> commandList)
	{
		Engine::GPUTimer::BeginEvent(commandList, "Debug Renderer");
		for (uint32 i = 0; i < s_DebugData.NumPasses; i++)
		{
			Pass* pass = s_DebugData.Passes[i];
			Utils::Vector<DebugVertex>& verts = pass->Vertices;
			Utils::Vector<uint32>& indices = pass->Indices;

			if (!verts.Empty())
			{
				commandList->SetShader(pass->m_Pass);
				commandList->SetConstantBuffer(0, s_DebugData.Camera);
				commandList->DrawMesh(pass->m_Mesh);
			}

			verts.Clear();
			indices.Clear();
		}
		Engine::GPUTimer::EndEvent(commandList);
	}

	void DebugRenderer::DrawLine(Math::Vector3 p1, Math::Vector3 p2, Math::Vector4 color, const Math::Mat4& transform, bool depthTest)
	{
		DebugVertex vert;
		vert.Color = color;

		Pass& pass = depthTest ? s_DebugData.DepthTestLinesPass : s_DebugData.LinesPass;

		uint32 baseVertexIndex = (uint32)pass.Vertices.Count();
		pass.Indices.Push(baseVertexIndex);
		pass.Indices.Push(baseVertexIndex + 1);

		vert.Position = transform * Math::Vector4(p1, 1.0f);
		pass.Vertices.Push(vert);
		vert.Position = transform * Math::Vector4(p2, 1.0f);
		pass.Vertices.Push(vert);
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
			{{-1,  1, -1, 1}, color}, // 0
			{{ 1,  1, -1, 1}, color}, // 1
			{{ 1, -1, -1, 1}, color}, // 2
			{{-1, -1, -1, 1}, color}, // 3
			{{-1,  1,  1, 1}, color}, // 4
			{{ 1,  1,  1, 1}, color}, // 5
			{{ 1, -1,  1, 1}, color}, // 6
			{{-1, -1,  1, 1}, color}, // 7
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
			{{-1,  1, -1, 1}, color}, // 0
			{{ 1,  1, -1, 1}, color}, // 1
			{{ 1, -1, -1, 1}, color}, // 2
			{{-1, -1, -1, 1}, color}, // 3
			{{-1,  1,  1, 1}, color}, // 4
			{{ 1,  1,  1, 1}, color}, // 5
			{{ 1, -1,  1, 1}, color}, // 6
			{{-1, -1,  1, 1}, color}, // 7
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


	void DebugRenderer::DrawFrustom(const Math::Mat4& vp, Math::Vector4 color, bool depthTest /*= true*/)
	{
		Engine::DebugMesh mesh;
		mesh.m_Vertices = {
			{{-1,  1,  0, 1}, color}, // 0
			{{ 1,  1,  0, 1}, color}, // 1
			{{ 1, -1,  0, 1}, color}, // 2
			{{-1, -1,  0, 1}, color}, // 3
			{{-1,  1,  1, 1}, color}, // 4
			{{ 1,  1,  1, 1}, color}, // 5
			{{ 1, -1,  1, 1}, color}, // 6
			{{-1, -1,  1, 1}, color}, // 7
		};

		mesh.m_Indices = {
			0,4, 1,5, 2,6, 3,7,
			0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4
		};

		DrawLineMesh(mesh, Math::Inverse(vp), depthTest);
	}

	void DebugRenderer::DrawMesh(const DebugMesh& mesh, const Math::Mat4& transform, Pass& pass)
	{
		uint32 baseVertexIndex = (uint32)pass.Vertices.Count();
		for (uint32 i = 0; i < mesh.m_Vertices.Count(); i++)
		{
			DebugVertex vert = mesh.m_Vertices[i];
			vert.Position = transform * vert.Position;
			vert.Position = vert.Position / vert.Position.w;
			pass.Vertices.Push(vert);
		}

		for (uint32 i = 0; i < mesh.m_Indices.Count(); i++)
			pass.Indices.Push(baseVertexIndex + mesh.m_Indices[i]);
	}

}
