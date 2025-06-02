#pragma once
#include "Utils/Time.h"

#include "Engine/Core/UUID.h"

#include "Engine/Renderer/SceneRenderer.h"
#include "SceneRegistry.h"
#include "ComponentPool.h"

#include "Engine/AssetManager/AssetManager.h"

namespace Engine
{
	class Entity;
	class Camera;
	class SceneScriptBase;
	class SceneSystem;
}

namespace Engine
{
	class Scene;

	class UpdateEvent
	{
	public:
		virtual ~UpdateEvent() = default;
		virtual void Setup(Scene* scene) { m_Scene = scene; };
		virtual void Update() = 0;
	protected:
		Scene* m_Scene;
	};



	class Scene : public Asset
	{
	public:
		Scene(SceneScriptBase* script = nullptr);
		~Scene();

		void OnUpdate();
		void OnViewportResize(uint32 width, uint32 height);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(const UUID uuid, const std::string& name = "");

		void DestroyEntity(Entity entity);

		Entity GetEntityWithUUID(UUID id);

		uint32 GetViewportWidth() { return m_ViewportWidth; }
		uint32 GetViewportHeight() { return m_ViewportHeight; }

		// -------------------------------- ECS -------------------------------- //
		SceneRegistry& GetRegistry() { return m_Registry; }
		template<class T>
		T* GetSceneStatic()
		{
			return m_Registry.GetSceneStatic<T>();
		}
		// -------------------------------- ECS -------------------------------- //


		// -------------------------------- Rendering -------------------------------- //
		template<class T>
		Ref<T> GetSceneRendererAs() { return std::dynamic_pointer_cast<T>(m_SceneRenderer); }
		Ref<SceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }
		// -------------------------------- Rendering -------------------------------- //


		// -------------------------------- Systems -------------------------------- //
		template<class T> 
		T* GetSceneSystem() {
			return (T*)GetSceneSystem(T::GetStaticClass());
		}
		SceneSystem* GetSceneSystem(const Reflect::Class& systemClass) { return m_SystemsMap[systemClass.GetTypeID()]; }
		const Utils::Vector<SceneSystem*>& GetSceneSystems() const { return m_Systems; }

		template<class T>
		T* CreateSceneSystem() { return (T*)CreateSceneSystem(T::GetStaticClass()); }
		SceneSystem* CreateSceneSystem(const Reflect::Class& systemClass);
		Utils::Vector<SceneSystem*> CreateSceneSystems(const Utils::Vector<const Reflect::Class*>& systemClasses);

		template<class T>
		void RemoveSceneSystem() { RemoveSceneSystem(T::GetStaticClass()); }
		void RemoveSceneSystem(const Reflect::Class& systemClass);
		// -------------------------------- Systems -------------------------------- //

		SceneScriptBase* GetSceneScript() { return m_SceneScript; }

		void RegenerateUpdateEvents();

		template<class T, typename ...Args>
		void AddUpdateEvent(Args... args) { 
			T* event = new T(std::forward<Args>(args)...);
			m_UpdateEvents.push_back(event);
			event->Setup(this);
		}

		static Ref<Scene> Create();
		static Ref<Scene> Create(const fs::path& file);
		static Ref<Scene> CreateDefault();

	private:
		uint32 m_ViewportWidth = 0;
		uint32 m_ViewportHeight = 0;
		SceneRegistry m_Registry;
		Utils::Vector<SceneSystem*> m_Systems;
		std::unordered_map<uint64, SceneSystem*> m_SystemsMap;
		SceneScriptBase* m_SceneScript;

		Ref<SceneRenderer> m_SceneRenderer;

		Utils::Vector<UpdateEvent*> m_UpdateEvents;

		bool m_CameraChanged = false;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneScriptBase;
	};





	class SceneUpdateEvent : public UpdateEvent
	{
	public:
		SceneUpdateEvent(const std::string& funcName);
		virtual void Setup(Scene* scene) override;
		virtual void Update() override;

	private:
		Utils::Vector<ComponentPool*> m_Pools;
		Utils::Vector<const Reflect::Function*> m_Funcs;
		const std::string m_FuncName;
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
				CREATE_PROFILE_SCOPEI("Component Update Event");
				ANOTATE_PROFILEI(m_FuncName);
				m_Pool->EachEntity([this](void* component) {
					m_Func->Invoke(component, {});
				});
			}
		}

	private:
		ComponentPool* m_Pool;
		const Reflect::Function* m_Func;
		const std::string m_FuncName;
	};

	template<typename T>
	class SceneSystemUpdateEvent : public UpdateEvent
	{
	public:
		SceneSystemUpdateEvent(const std::string& funcName) :
			m_FuncName(funcName)
		{}

		virtual void Setup(Scene* scene) override {
			const Reflect::Class& systemClass = T::GetStaticClass();
			m_System = scene->GetSceneSystem(systemClass);
			if (systemClass.HasFunction(m_FuncName))
				m_Func = &systemClass.GetFunction(m_FuncName);
		}

		virtual void Update() override {
			if (m_Func && m_System) {
				CREATE_PROFILE_SCOPEI("Scene Subsystem Update Event");
				ANOTATE_PROFILEI(m_FuncName);
				m_Func->Invoke(m_System, {});
			}
		}


	private:
		SceneSystem* m_System;
		const Reflect::Function* m_Func;
		std::string m_FuncName;
	};

}
