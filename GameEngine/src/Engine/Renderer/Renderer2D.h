#pragma once

#include <glm/glm.hpp>
#include "Texture.h"
#include "SubTexture.h"

namespace Engine
{
	class OrthographicCamera;
}

namespace Engine
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		// draw colored quaad
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

		// draw sub textured quad
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& texture);

		// draw sub textured tinted quad
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color, const Ref<SubTexture2D>& texture);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, const Ref<SubTexture2D>& texture);

		// draw textured qaud
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture);

		// draw textured tinted qaud
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& texture);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();

	private:

		static void DrawQuadImpl(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color, const Ref<Texture2D>& texture, const glm::vec2 textCoords[]);
		static void DrawQuadImpl(const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& texture, const glm::vec2 textCoords[]);

		static void BeginBatch();
	};
}