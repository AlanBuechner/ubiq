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
	class Scene;

	class UpdateEvent
	{
	public:
		virtual void Setup(Scene* scene) { m_Scene = scene; };
		virtual void Update() = 0;
	protected:
		Scene* m_Scene;
	};

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

		template<class T, typename ...Args>
		void AddUpdateEvent(Args... args) { 
			T* event = new T(std::forward<Args>(args)...);
			m_UpdateEvents.push_back(event);
			event->Setup(this);
		}

	private:
		uint32 m_ViewportWidth = 0;
		uint32 m_ViewportHeight = 0;
		SceneRegistry m_Registry;

		Ref<SceneRenderer> m_SceneRenderer;

		std::vector<UpdateEvent*> m_UpdateEvents;

		bool m_CameraChanged = false;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class Physics2D;
	};





	class SceneUpdateEvent : public UpdateEvent
	{
	public:
		SceneUpdateEvent(const std::string& funcName);
		virtual void Setup(Scene* scene) override;
		virtual void Update() override;

	private:
		std::vector<ComponentPool*> m_Pools;
		std::vector<const Reflect::Function*> m_Funcs;
		std::string m_FuncName;
	};

	template<typename T>
	class ComponentUpdateEvent : public UpdateEvent
	{
	public:
		ComponentUpdateEvent(const std::string& funcName) :
			m_FuncName(funcName)
		{}

		virtual void Setup(Scene* scene) override {
			const Reflect::Class& componentClass = T::GetStaticClass();
			m_Pool = scene->GetRegistry().GetOrCreateCompnentPool(componentClass);
			if (componentClass.HasFunction(m_FuncName))
				m_Func = &componentClass.GetFunction(m_FuncName);
		}

		virtual void Update() override {
			if (m_Func)
			{
				CREATE_PROFILE_SCOPEI(m_FuncName);
				m_Pool->EachEntity([this](void* component) {
					m_Func->Invoke(component, {});
				});
			}
		}

	private:
		ComponentPool* m_Pool;
		const Reflect::Function* m_Func;
		std::string m_FuncName;
	};
}
