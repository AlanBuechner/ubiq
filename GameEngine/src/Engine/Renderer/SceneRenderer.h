#pragma once
#include "Engine/Core/Core.h"
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
	class Texture;
}

namespace Engine
{

	class SceneRenderer
	{
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
			Ref<InstanceBuffer> m_Instances;
			Ref<Mesh> m_Mesh;

			std::list<ObjectControlBlock> m_ControlBlocks;

			ObjectControlBlockRef AddInstance(const Math::Mat4& transform, Ref<Material> mat);
			void RemoveInstance(ObjectControlBlockRef controlBlock);
			
		};

		class ShaderDrawSection
		{
		public:
			using iterator = std::list<RenderObject>::iterator;

			Ref<Shader> m_Shader;

			iterator begin() { return m_Objects.begin(); }
			iterator end() { return m_Objects.end(); }

			SceneRenderer::ObjectControlBlockRef AddObject(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform);

			std::list<RenderObject> m_Objects;
		};

	public:
		SceneRenderer();

		Ref<FrameBuffer> GetRenderTarget();
		void OnViewportResize(uint32 width, uint32 height);
		void SetMainCamera(Ref<Camera> camera);
		void SetDirectionalLight(Ref<DirectionalLight> light);
		void SetSkyBox(Ref<Texture> texture);
		void UpdateBuffers();
		void Invalidate();

		ObjectControlBlockRef Submit(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform);
		void RemoveObject(ObjectControlBlockRef controlBlock);

		void Build();
		void Render(Ref<CommandQueue> queue);

		static Ref<SceneRenderer> Create();

	private:
		Ref<Camera> m_MainCamera;
		std::vector<ShaderDrawSection> m_ShaderDrawSection;

		bool m_Invalid = true;

		Ref<RenderGraph> m_RenderGraph;
	};
}
