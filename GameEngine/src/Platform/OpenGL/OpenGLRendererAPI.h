#pragma once
#include "Engine/Renderer/RendererAPI.h"

namespace Engine
{
	class OpenGLRendererAPI : public RendererAPI
	{
		virtual void Init() override;
		virtual void SetViewPort(uint32 x, uint32 y, uint32 width, uint32 height) override;
		virtual void SetClearColor(const Math::Vector4 & color) override;
		virtual void Clear() override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32 indexCount = 0) override;
		virtual void DrawLineIndexed(const Ref<VertexArray>& vertexArray, uint32 indexCount = 0) override;
	};
}
