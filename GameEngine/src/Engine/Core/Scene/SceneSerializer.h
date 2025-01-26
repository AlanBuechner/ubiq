#pragma once
#include "Scene.h"

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

	private:
		Ref<Scene> m_Scene;
	};
}
