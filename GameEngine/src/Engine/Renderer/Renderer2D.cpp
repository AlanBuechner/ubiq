#include "pch.h"
#include "Renderer2D.h"

#include "Engine/Core/Core.h"
#include "Renderer.h"
#include "Abstractions/Resources/InstanceBuffer.h"
#include "Abstractions/Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Engine/Core/Scene/Entity.h"
#include "Engine/Core/Scene/Components.h"

#include "Engine/Util/Performance.h"

#include "EngineResource.h"
#include "Engine/Renderer/Mesh.h"

namespace Engine
{
	struct QuadVertex
	{
		Math::Vector3 Position;
		Math::Vector2 TexCoord;
	};

	struct QuadInstance
	{
		Math::Mat4 Transform;
		Math::Vector4 Color;
		uint32 TextureID;
		Math::Vector2 UVSize;
		Math::Vector2 UVPosition;

		int EntityID;
	};

	struct Renderer2DData
	{
		static const uint32 MaxQuades = 100;
		Ref<InstanceBuffer> QuadInstanceBuffer;

		Ref<Mesh> QuadMesh;
		Ref<Shader> SpriteShader;

		Ref<ConstantBuffer> Camera;

		Renderer2D::Statistics Stats;

		void AddInstance(const QuadInstance& instance)
		{
			QuadInstanceBuffer->PushBack(&instance);
		}
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		// create mesh
		s_Data.QuadMesh = Engine::Mesh::Create(sizeof(QuadVertex));
		QuadVertex verts[] = {
			{ {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} },
			{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} },
			{ { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} },
			{ { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} },
		};
		s_Data.QuadMesh->SetVertices(verts, _countof(verts));
		uint32 indices[] = {
			0,1,2,2,1,3
		};
		s_Data.QuadMesh->SetIndices(indices, _countof(indices));

		// create shader
		s_Data.SpriteShader = Shader::CreateFromEmbeded(TEXTURE, "TextureShader.hlsl");

		// create instance data
		s_Data.QuadInstanceBuffer = InstanceBuffer::Create(sizeof(QuadInstance), s_Data.MaxQuades);

		s_Data.Camera = ConstantBuffer::Create(sizeof(Math::Mat4));
	}

	void Renderer2D::Destroy()
	{
		// destroy all gpu assets
		s_Data.QuadInstanceBuffer.reset();
		s_Data.Camera.reset();
		s_Data.QuadMesh.reset();
		s_Data.SpriteShader.reset();
	}

	void Renderer2D::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		Math::Mat4 viewProj = camera.GetProjectionMatrix() * Math::Inverse(transform);
		s_Data.Camera->SetData(&viewProj);
	}

	void Renderer2D::BeginScene(Ref<EditorCamera> camera)
	{
		Math::Mat4 viewProj = camera->GetViewProjection();
		s_Data.Camera->SetData(&viewProj);
	}

	void Renderer2D::EndScene()
	{
		s_Data.QuadInstanceBuffer->Apply();
	}

	void Renderer2D::Build(Ref<CommandList> commandList)
	{
		if (!s_Data.QuadInstanceBuffer->Empty())
		{
			commandList->SetShader(s_Data.SpriteShader->GetPass("main"));
			commandList->SetConstantBuffer(0, s_Data.Camera);
			commandList->DrawMesh(s_Data.QuadMesh, s_Data.QuadInstanceBuffer, s_Data.QuadInstanceBuffer->GetCount());
		}
		s_Data.QuadInstanceBuffer->Clear();
	}

	void Renderer2D::ResetStats()
	{
		//memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}


	// draw colored quad --------------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, Renderer::GetWhiteTexture(), { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, Renderer::GetWhiteTexture(), { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, Renderer::GetWhiteTexture(), { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, Renderer::GetWhiteTexture(), { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(transform, color, Renderer::GetWhiteTexture(), { 1,1 }, { 0,0 }, entityID);
	}



	// draw sub textured quad ---------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	/*void Renderer2D::DrawQuad(const Math::Mat4& transform, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}*/



	// draw sub texture tinted quad ----------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, color, texture->GetTexture(), texture->GetSize(), texture->GetPosition(), entityID);
	}



	// draw textured quad --------------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, { 1,1 }, { 0,0 }, entityID);
	}
	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, { 1,1 }, { 0,0 }, entityID);
	}



	// draw textured tinted quad
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, texture, { 1,1 }, { 0,0 }, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, color, texture, { 1,1 }, { 0,0 }, entityID);
	}



	// draw quad implementation
	void Renderer2D::DrawQuadImpl(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2& uvSize, const Math::Vector2& uvPosition, int entityID)
	{
		Math::Mat4 transform = Math::Translate(position) *
			Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) *
			Math::Scale({ size.x, size.y, 1.0f });

		DrawQuadImpl(transform, color, texture, uvSize, uvPosition);
	}

	void Renderer2D::DrawQuadImpl(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2& uvSize, const Math::Vector2& uvPosition, int entityID)
	{
		QuadInstance i;
		i.Transform = transform;
		i.Color = color;
		i.TextureID = texture->GetSRVDescriptor()->GetIndex();
		i.EntityID = entityID;
		i.UVSize = uvSize;
		i.UVPosition = uvPosition;
		s_Data.AddInstance(i);
	}

}
