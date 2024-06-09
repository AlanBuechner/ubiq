#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/MeshBuilder.h"
#include "Engine/Physics/AABB.h"

namespace Engine
{
	class Mesh;
}

namespace Game
{

	class Model : public Engine::Asset
	{
	public:
		struct Node
		{
			Utils::Vector<Engine::MeshBuilder> m_MeshBuilders;
			Utils::Vector<std::string> m_Names;
			Utils::Vector<Node> m_Children;
			Math::Mat4 m_Transform;
		};

	public:
		Model() = default;

		void BakeMesh();

		Node& GetRoot() { return m_Root; }
		Utils::Vector<Engine::Ref<Engine::Mesh>>& GetBakedMeshes() { return m_BakedMeshes; }
		Utils::Vector<std::string>& GetNames() { return m_Names; }
		Utils::Vector<Engine::AABB>& GetVolumes() { return m_Volumes; }

		static Engine::Ref<Model> Create(const fs::path& path);

		static bool ValidExtension(const fs::path& ext);

	private:

		void CollapseNode(Node& node, Math::Mat4 parentTransform);

	private:

		Node m_Root;

		Utils::Vector<Engine::Ref<Engine::Mesh>> m_BakedMeshes;
		Utils::Vector<Engine::AABB> m_Volumes;
		Utils::Vector<std::string> m_Names;
	};

}
