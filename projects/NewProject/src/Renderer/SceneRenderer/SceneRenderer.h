#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/SceneRenderer.h"
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
	class DirectionalLight;
	class Shader;
	class Texture2D;
}

namespace Game
{

	class SceneRenderer : public Engine::SceneRenderer
	{
	private:
		struct RenderObject;

	public:
		struct ObjectControlBlock
		{
			RenderObject& m_Object;
			uint32 m_InstanceLocation;

			void UpdateTransform(const Math::Mat4& transform);
		};

		using ObjectControlBlockRef = ObjectControlBlock*;

	private:
		struct InstanceData
		{
			Math::Mat4 m_Transform;
			uint32 m_MaterialIndex;
		};

		struct RenderObject
		{
			RenderObject();
			Engine::Ref<Engine::InstanceBuffer> m_Instances;
			Engine::Ref<Engine::Mesh> m_Mesh;

			std::list<ObjectControlBlock> m_ControlBlocks;

			ObjectControlBlockRef AddInstance(const Math::Mat4& transform, Engine::Ref<Engine::Material> mat);
			void RemoveInstance(ObjectControlBlockRef controlBlock);
			
		};

		class ShaderDrawSection
		{
		public:
			using iterator = std::list<RenderObject>::iterator;

			Engine::Ref<Engine::Shader> m_Shader;

			iterator begin() { return m_Objects.begin(); }
			iterator end() { return m_Objects.end(); }

			SceneRenderer::ObjectControlBlockRef AddObject(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform);

			std::list<RenderObject> m_Objects;
		};

	public:
		SceneRenderer();

		
		virtual void OnViewportResize(uint32 width, uint32 height) override;
		virtual void UpdateBuffers() override;

		void SetDirectionalLight(Engine::Ref<Engine::DirectionalLight> light);
		void SetSkyBox(Engine::Ref<Engine::Texture2D> texture);

		ObjectControlBlockRef Submit(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform);
		void RemoveObject(ObjectControlBlockRef controlBlock);

		virtual void Build() override;
		virtual void Render(Engine::Ref<Engine::CommandQueue> queue) override;

		static Engine::Ref<SceneRenderer> Create();

	private:
		
		std::vector<ShaderDrawSection> m_ShaderDrawSection;
	};
}
