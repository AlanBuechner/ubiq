#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Window.h"
#include "Engine/Renderer/Resources/FrameBuffer.h"

namespace Engine
{
	class SwapChain
	{
	public:
		virtual void Init(uint32 numBuffers) = 0;
		virtual void Resize(uint32 width, uint32 height) = 0;
		virtual void SetVSync(bool val) = 0;
		virtual void SwapBuffers() = 0;
		virtual void UpdateBackBufferIndex() = 0;
		virtual void CleanUp() = 0;
		virtual uint32 GetBufferCount() = 0;

		virtual Ref<RenderTarget2D> GetCurrentRenderTarget() = 0;

		static Ref<SwapChain> Create(Window& window);
	};
}
