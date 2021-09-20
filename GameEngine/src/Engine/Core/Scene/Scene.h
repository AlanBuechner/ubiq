#pragma once
#include "entt.hpp"
#include "Engine/Core/Time.h"

#include "Engine/Renderer/Texture.h"

class b2World;

namespace Engine
{
	class Entity;
	class EditorCamera;
}

namespace Engine
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateEditor(const EditorCamera& camera);
		void OnUpdateRuntime();
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		entt::registry m_Registry;

		b2World* m_World = nullptr;

		Ref<Texture2D> m_CameraIcon;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}