#pragma once
#include "Engine\Core\Core.h"
#include "GraphicsContext.h"
#include "CommandQueue.h"

#include "Material.h"

#include "EditorCamera.h"
#include "Light.h"

#include "Engine/Util/Performance.h"

#include "Engine/Core/Flag.h"
#include <thread>

#define MAX_LIGHTS 12

namespace Engine
{
	class Camera;
	class ShaderPass;
}

namespace Engine
{
	enum class RendererAPI
	{
		None = 0,
		DirectX12 = 1
	};

	class Renderer
	{
	public:
		static void Init();
		static void Destroy();

		static void BeginFrame();
		static void EndFrame();
		static void WaitForRender();
		static void WaitForSwap();

		static void Build(Ref<CommandList> commandList);

		static Ref<Texture2D> GetWhiteTexture() { return s_WhiteTexture; }
		static Ref<Texture2D> GetBlackTexture() { return s_BlackTexture; }
		static Ref<Texture2D> GetBlueTexture() { return s_BlueTexture; }
		static Ref<Mesh> GetScreenMesh() { return s_ScreenMesh; }
		static Ref<ComputeShader> GetBlitShader() { return s_BlitShader; }

		inline static RendererAPI GetAPI() { return s_Api; }
		inline static Ref<GraphicsContext> GetContext() { return s_Context; }
		template<class T>
		inline static Ref<T> GetContext() { return std::dynamic_pointer_cast<T>(GetContext()); }

		inline static Ref<CommandQueue> GetMainCommandQueue() { return s_MainCommandQueue; }
		template<class T>
		inline static Ref<T> GetMainCommandQueue() { return std::dynamic_pointer_cast<T>(GetMainCommandQueue()); }

		inline static Ref<CommandList> GetMainCommandList() { return s_MainCommandList; }
		template<class T>
		inline static Ref<T> GetMainCommandList() { return std::dynamic_pointer_cast<T>(s_MainCommandList); }

	private:
		static void Render();

	private:
		static Ref<GraphicsContext> s_Context;
		static Ref<CommandQueue> s_MainCommandQueue;
		static Ref<CommandList> s_MainCommandList;
		static Ref<CommandQueue> s_MainCopyCommandQueue;
		static Ref<CommandList> s_MainCopyCommandList;

		static RendererAPI s_Api;

		static InstrumentationTimer s_Timer;

		static std::thread s_RenderThread;

		static Flag s_RenderFlag;
		static Flag s_SwapFlag;
		static Flag s_CopyFlag;

		static Ref<Texture2D> s_WhiteTexture;
		static Ref<Texture2D> s_BlackTexture;
		static Ref<Texture2D> s_BlueTexture;
		static Ref<Mesh> s_ScreenMesh;
		static Ref<ComputeShader> s_BlitShader;
	};
}
