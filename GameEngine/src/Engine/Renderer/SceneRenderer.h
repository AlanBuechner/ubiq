#pragma once
#include "Engine/Core/Core.h"
#include "Mesh.h"
#include "Material.h"
#include "InstanceBuffer.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "ConstantBuffer.h"
#include <vector>
#include <list>

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

		struct DrawCommand
		{
			Ref<Shader> m_Shader;
			Ref<Mesh> m_Mesh;
			Ref<InstanceBuffer> m_InstanceBuffer;
		};

		struct RenderObject
		{
			Ref<InstanceBuffer> m_Instances = InstanceBuffer::Create(sizeof(InstanceData), 10); // default to capacity of 10
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

		void BeginScene(const Camera& camera, const Math::Mat4& transform);
		void BeginScene(const EditorCamera& camera);
		void Invalidate();

		ObjectControlBlockRef Submit(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform);
		void RemoveObject(ObjectControlBlockRef controlBlock);

		void Build();

		static Ref<SceneRenderer> Create();

	private:
		std::vector<DrawCommand> m_DrawCommands;

		std::vector<ShaderDrawSection> m_ShaderDrawSection;

		Math::Mat4 m_ViewPorj;
		Ref<ConstantBuffer> m_CameraBuffer;
	};
}
