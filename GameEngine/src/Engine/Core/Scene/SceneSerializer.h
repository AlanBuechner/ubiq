#pragma once
#include "Scene.h"
#include "Engine/Core/ObjectDescription/ObjectDescription.h"

namespace Engine
{
	class Component;
}

namespace Engine
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const fs::path& filepath);
		void SerializeRuntime(const fs::path& filepath);

		bool Deserialize(const fs::path& filepath);
		bool DeserializeRuntime(const fs::path& filepath);

		static ObjectDescription GetComponentObjectDescription(Component* comp);

	private:
		Ref<Scene> m_Scene;
	};
}
