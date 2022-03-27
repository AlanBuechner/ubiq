#include "pch.h"
#include "Renderer2D.h"

#include "Engine/Core/core.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Engine/Core/Scene/Components.h"

#include "Engine/Util/Performance.h"

namespace Engine
{
	struct QuadVertex
	{
		Math::Vector3 Position;
		Math::Vector4 Color;
		Math::Vector2 TexCoord;
		float TexIndex;

		// editor only
		int EntityID;
	};

	struct Renderer2DData
	{
		static const uint32_t QuadVertexCount = 4;
		static const uint32_t MaxQuades = 10000;
		static const uint32_t MaxVertices = MaxQuades * QuadVertexCount;
		static const uint32_t MaxIndices = MaxQuades * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: RendererCaps

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<ShaderLibrary> Library;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		Math::Vector4 QuadVertexPositions[4];

		Math::Vector2 TextureCoords[4];

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		s_Data.QuadVertexArray = Engine::VertexArray::Create();

		s_Data.QuadVertexBuffer = Engine::VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float4, "a_Color"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"},
			{Engine::ShaderDataType::Float, "a_TexIndex"},
			{Engine::ShaderDataType::Int, "a_EntityID"}
		};

		s_Data.QuadVertexBuffer->SetLayout(layout);

		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndeces = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndeces[i + 0] = offset + 0;
			quadIndeces[i + 1] = offset + 1;
			quadIndeces[i + 2] = offset + 2;

			quadIndeces[i + 3] = offset + 2;
			quadIndeces[i + 4] = offset + 3;
			quadIndeces[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = Engine::IndexBuffer::Create(quadIndeces, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);

		delete[] quadIndeces;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTexureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTexureData, sizeof(whiteTexureData));

		int32_t samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_Data.Library = std::make_shared<ShaderLibrary>();

		Ref<Shader> TextureShader = s_Data.Library->Load("TextureShader", "Assets/Shaders/TextureShader.glsl");

		TextureShader->Bind();
		TextureShader->UploadUniformIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		//TextureShader->UploadUniformInt("u_Texture", 0);

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.TextureCoords[0] = { 0,0 };
		s_Data.TextureCoords[1] = { 1,0 };
		s_Data.TextureCoords[2] = { 1,1 };
		s_Data.TextureCoords[3] = { 0,1 };
	}

	void Renderer2D::Shutdown()
	{
		
	}

	void Renderer2D::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		CREATE_PROFILE_FUNCTIONI();

		Math::Mat4 viewProj = camera.GetProjectionMatrix() * glm::inverse(transform);

		Ref<Shader> TextureShader = s_Data.Library->Get("TextureShader");
		TextureShader->Bind();
		TextureShader->UploadUniformMat4("u_ViewProjection", viewProj);
		s_Data.QuadVertexArray->Bind();

		BeginBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		CREATE_PROFILE_FUNCTIONI();

		Math::Mat4 viewProj = camera.GetViewProjection();

		Ref<Shader> TextureShader = s_Data.Library->Get("TextureShader");
		TextureShader->Bind();
		TextureShader->UploadUniformMat4("u_ViewProjection", viewProj);
		s_Data.QuadVertexArray->Bind();

		BeginBatch();
	}

	void Renderer2D::EndScene()
	{
		CREATE_PROFILE_FUNCTIONI();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		CREATE_PROFILE_FUNCTIONI();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			s_Data.TextureSlots[i].get()->Bind(i);
		}

		if(s_Data.QuadIndexCount != 0)
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer2D::BeginBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}


	// draw colored quad --------------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, s_Data.WhiteTexture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, s_Data.WhiteTexture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, s_Data.WhiteTexture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, s_Data.WhiteTexture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, int entityID)
	{
		DrawQuadImpl(transform, color, s_Data.WhiteTexture, s_Data.TextureCoords, entityID);
	}



	// draw sub textured quad ---------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	/*void Renderer2D::DrawQuad(const Math::Mat4& transform, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, { 1.0f, 1.0f, 1.0f, 1.0f }, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}*/



	// draw sub texture tinted quad ----------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, color, texture->GetTexture(), texture->GetTextCordes(), entityID);
	}



	// draw textured quad --------------------------------------
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, s_Data.TextureCoords, entityID);
	}
	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, s_Data.TextureCoords, entityID);
	}



	// draw textured tinted quad
	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, 0.0f, color, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl({ position.x, position.y, 0.0f }, size, rotation, color, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, 0.0f, color, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(position, size, rotation, color, texture, s_Data.TextureCoords, entityID);
	}

	void Renderer2D::DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID)
	{
		DrawQuadImpl(transform, color, texture, s_Data.TextureCoords, entityID);
	}



	// draw sprite component
	void Renderer2D::DrawSprite(const Math::Mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		DrawQuadImpl(transform, src.Color, (src.Texture ? src.Texture : s_Data.WhiteTexture), s_Data.TextureCoords, entityID);
	}



	// draw quad implementation
	void Renderer2D::DrawQuadImpl(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2 textCoords[], int entityID)
	{
		CREATE_PROFILE_FUNCTIONI();


		Math::Mat4 transform = glm::translate(Math::Mat4(1.0f), position) *
			glm::rotate(Math::Mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(Math::Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuadImpl(transform, color, texture, textCoords);
	}

	void Renderer2D::DrawQuadImpl(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2 textCoords[], int entityID)
	{

		CREATE_PROFILE_FUNCTIONI();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{
			EndScene();
			BeginBatch();
		}

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (s_Data.TextureSlots[i].get() == texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		for (uint32_t i = 0; i < s_Data.QuadVertexCount; i++)
		{

			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

}
