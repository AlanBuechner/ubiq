#pragma once
#include "Engine/Core/Time.h"

#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Core/UUID.h"

#include "Engine/Renderer/SceneRenderer.h"
#include "SceneRegistry.h"
#include "ComponentPool.h"

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

		void OnUpdateEditor(Ref<EditorCamera> camera);
		void OnUpdateRuntime();
		void OnViewportResize(uint32 width, uint32 height);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(const UUID uuid, const std::string& name = "");
		void DestroyEntity(Entity entity);

		Entity GetEntityWithUUID(UUID id);

		Entity GetPrimaryCameraEntity();

		uint32 GetViewportWidth() { return m_ViewportWidth; }
		uint32 GetViewportHeight() { return m_ViewportHeight; }

		template<class T>
		T* GetSceneStatic()
		{
			return m_Registry.GetSceneStatic<T>();
		}

		Ref<SceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }
		SceneRegistry& GetRegistry() { return m_Registry; }

		static Ref<Scene> Copy(Ref<Scene> scene);

	private:
		uint32 m_ViewportWidth = 0;
		uint32 m_ViewportHeight = 0;
		SceneRegistry m_Registry;

		Ref<SceneRenderer> m_SceneRenderer;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class Physics2D;
	};
}
