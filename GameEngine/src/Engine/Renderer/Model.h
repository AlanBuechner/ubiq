#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Engine/Math/Math.h"

namespace Engine
{
	class Mesh;
}

namespace Engine
{

	class Model : public Asset
	{
	public:
		Model() = default;

		void AddMesh(Ref<Mesh> mesh) { m_Meshes.push_back(mesh); }
		void AddChild(Ref<Model> child) { m_Children.push_back(child); }
		void SetTransform(Math::Mat4 transform) { m_Transform = transform; }

		std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
		std::vector<Ref<Model>>& GetChildren() { return m_Children; }
		Math::Mat4 GetTransform() { return m_Transform; }

		static Ref<Model> Create(const fs::path& path);

		static bool ValidExtension(const fs::path& ext);

	private:

		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Ref<Model>> m_Children;
		Math::Mat4 m_Transform = Math::Mat4(1.0f);
	};

}
