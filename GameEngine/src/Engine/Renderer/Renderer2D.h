#pragma once

#include <Engine/Math/Math.h>
#include "Texture.h"
#include "SubTexture.h"
#include "CommandList.h"

namespace Engine
{
	class Camera;
	class EditorCamera;

	struct SpriteRendererComponent;
}

namespace Engine
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Destroy();

		static void BeginScene(const Camera& camera, const Math::Mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Build(Ref<CommandList> commandList);

		// draw colored quad
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, int entityID = -1);
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, int entityID = -1);
		static void DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, int entityID = -1);

		// draw sub textured quad
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<SubTexture2D>& texture, int entityID = -1);
		//static void DrawQuad(const Math::Mat4& transform, const Ref<SubTexture2D>& texture, int entityID = -1);

		// draw sub textured tinted quad
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<SubTexture2D>& texture, int entityID = -1);

		// draw textured quad
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Mat4& transform, const Ref<Texture2D>& texture, int entityID = -1);

		// draw textured tinted quad
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector2& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID = -1);
		static void DrawQuad(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, int entityID = -1);

		// draw sprite component
		static void DrawSprite(const Math::Mat4& transform, SpriteRendererComponent& src, int entityID);

		struct Statistics
		{
			uint32 DrawCalls = 0;
			uint32 QuadCount = 0;

			uint32 GetTotalVertexCount() { return QuadCount * 4; }
			uint32 GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();

	private:

		static void DrawQuadImpl(const Math::Vector3& position, const Math::Vector2& size, float rotation, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2& uvSize, const Math::Vector2& uvPosition, int entityID = -1);
		static void DrawQuadImpl(const Math::Mat4& transform, const Math::Vector4& color, const Ref<Texture2D>& texture, const Math::Vector2& uvSize, const Math::Vector2& uvPosition, int entityID = -1);
	};
}
