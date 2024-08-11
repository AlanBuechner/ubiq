#pragma once
#include "Engine/Core/Core.h"
#include "Renderer/RenderGraph/RenderGraph.h"
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
	struct RenderObject;

	// control block
	struct ObjectControlBlock
	{
		RenderObject& m_Object;
		uint32 m_InstanceLocation;

		void UpdateTransform(const Math::Mat4& transform);
		void DestroySelf();
	};
	using ObjectControlBlockRef = ObjectControlBlock*;

	// renderer object
	struct RenderObject
	{
		RenderObject();
		Engine::Ref<Engine::InstanceBuffer> m_Instances;
		Engine::Ref<Engine::Mesh> m_Mesh;

		std::list<ObjectControlBlock> m_ControlBlocks;

		ObjectControlBlockRef AddInstance(const Math::Mat4& transform, Engine::Ref<Engine::Material> mat);
		void RemoveInstance(ObjectControlBlockRef controlBlock);

	};

	struct InstanceData
	{
		Math::Mat4 m_Transform;
		uint32 m_MaterialIndex;
	};

	class ShaderDrawSection
	{
	public:
		using iterator = std::list<RenderObject>::iterator;

		Engine::Ref<Engine::Shader> m_Shader;

		iterator begin() { return m_Objects.begin(); }
		iterator end() { return m_Objects.end(); }

		ObjectControlBlockRef AddObject(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform);

		std::list<RenderObject> m_Objects;
	};


	class RenderPassObject
	{
	public:
		ObjectControlBlockRef Submit(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform);
		void RemoveObject(ObjectControlBlockRef controlBlock);

		void UpdateBuffers();
		void BuildDrawCommands(Utils::Vector<DrawCommand>& outDrawCommands);

	private:
		Utils::Vector<ShaderDrawSection> m_ShaderDrawSection;
	};
}
