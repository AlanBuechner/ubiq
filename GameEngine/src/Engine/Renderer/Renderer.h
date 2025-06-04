#pragma once
#include "Engine/Core/Core.h"
#include "Abstractions/GraphicsContext.h"
#include "Abstractions/CommandQueue.h"

#include "Material.h"

#include "Utils/Performance.h"

#include "Engine/Core/Threading/Flag.h"
#include "Engine/Core/Threading/JobSystem.h"

namespace Engine
{
	class Camera;
	class GraphicsShaderPass;
	class Texture2D;
	class Mesh;
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

		static void Build(Ref<CommandList> commandList);

		static void SetDefultMaterial(Ref<Material> mat) { s_DefultMaterial = mat; }

		static Ref<Texture2D> GetWhiteTexture() { return s_WhiteTexture; }
		static Ref<Texture2D> GetBlackTexture() { return s_BlackTexture; }
		static Ref<Texture2D> GetNormalTexture() { return s_NormalTexture; }
		static Ref<Mesh> GetScreenMesh() { return s_ScreenMesh; }
		static Ref<Shader> GetBlitShader() { return s_BlitShader; }
		static Ref<Material> GetDefultMaterial() { return s_DefultMaterial; }

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
		static Ref<CommandList> s_UploadCommandList;

		static RendererAPI s_Api;

		static Profiler::InstrumentationTimer s_Timer;

		static NamedJobThread* s_RenderThread;

		static Ref<Texture2D> s_WhiteTexture;
		static Ref<Texture2D> s_BlackTexture;
		static Ref<Texture2D> s_NormalTexture;
		static Ref<Mesh> s_ScreenMesh;
		static Ref<Shader> s_BlitShader;

		static Ref<Material> s_DefultMaterial;
	};
}
