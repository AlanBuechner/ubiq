#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "RenderingUtils/RenderPassObject.h"
#include <vector>
#include <list>

namespace Engine
{
	class CommandList;
	class CommandQueue;
	class Mesh;
	class Material;
	class InstanceBuffer;
	class Camera;
	class EditorCamera;
	class ConstantBuffer;
	class FrameBuffer;
	class RenderGraph;
	class Shader;
	class Texture2D;
}

namespace Game
{
	class DirectionalLight;
}

namespace Game
{

	class SceneRenderer : public Engine::SceneRenderer
	{
	public:
		SceneRenderer();

		RenderPassObject& GetMainPass() { return m_MainPassObject; }
		
		virtual void OnViewportResize(uint32 width, uint32 height) override;
		virtual void UpdateBuffers() override;

		void SetDirectionalLight(Engine::Ref<DirectionalLight> light);
		void SetSkyBox(Engine::Ref<Engine::Texture2D> texture);

		virtual void Build() override;

		static Engine::Ref<SceneRenderer> Create();

	private:
		RenderPassObject m_MainPassObject;
		
	};
}
