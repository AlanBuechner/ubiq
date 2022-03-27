#pragma once
#include "RendererAPI.h"

namespace Engine
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewPort(uint32 x, uint32 y, uint32 width, uint32 height)
		{
			s_RendererAPI->SetViewPort(x, y, width, height);
		}

		inline static void SetClearColor(const Math::Vector4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32 indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void DrawLineIndexed(const Ref<VertexArray>& vertexArray, uint32 indexCount = 0)
		{
			s_RendererAPI->DrawLineIndexed(vertexArray, indexCount);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}
