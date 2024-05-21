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
	class Camera;
}

namespace Engine
{
	class Scene
	{
	public:
		Scene();
		~Scene();


		void OnUpdate(Ref<Camera> camera);
		void OnViewportResize(uint32 width, uint32 height);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(const UUID uuid, const std::string& name = "");

		void DestroyEntity(Entity entity);

		Entity GetEntityWithUUID(UUID id);

		uint32 GetViewportWidth() { return m_ViewportWidth; }
		uint32 GetViewportHeight() { return m_ViewportHeight; }
		bool DidCameraChange() { return m_CameraChanged; }

		template<class T>
		T* GetSceneStatic()
		{
			return m_Registry.GetSceneStatic<T>();
		}

		Ref<SceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }
		SceneRegistry& GetRegistry() { return m_Registry; }

	private:
		uint32 m_ViewportWidth = 0;
		uint32 m_ViewportHeight = 0;
		SceneRegistry m_Registry;

		Ref<SceneRenderer> m_SceneRenderer;

		bool m_CameraChanged = false;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class Physics2D;
	};
}
