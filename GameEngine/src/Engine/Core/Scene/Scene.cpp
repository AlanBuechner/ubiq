#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "SceneSystem.h"
#include "SceneScriptBase.h"
#include "Engine/Core/UUID.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/DebugRenderer.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "SceneSerializer.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/Application.h"

#include "glm/glm.hpp"

namespace Engine
{
	Scene::Scene(SceneScriptBase* script) :
		m_SceneScript(script)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (script == nullptr)
			m_SceneScript = SceneScriptBase::GetDefultSceneScriptInstance();
		else
			m_SceneScript = script;

		m_SceneScript->m_Scene = this;
		m_SceneScript->OnScenePreLoad();
	}

	Scene::~Scene()
	{
		m_SceneRenderer = nullptr;
		for (uint32 i = 0; i < m_UpdateEvents.Count(); i++)
			delete m_UpdateEvents[i];
	}

	void Scene::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_SceneScript->OnUpdate();

		for (uint32 i = 0; i < m_UpdateEvents.Count(); i++)
			m_UpdateEvents[i]->Update();
		
		// render 3d models
		m_SceneRenderer->UpdateBuffers();
	}

	void Scene::OnViewportResize(uint32 width, uint32 height)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Registry.EachPool([&](ComponentPool* pool) {
			pool->EachEntity([&](void* component) {
				((Component*)component)->OnViewportResize(width, height);
			});
		});

		m_SceneRenderer->OnViewportResize(width, height);
	}

	Entity Scene::CreateEntity(const std::string& name /*= ""*/)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(const UUID uuid, const std::string& name)
	{
		Entity entity{ m_Registry.CreateEntity(uuid, name), this };
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.GetTransform().Parent)
			entity.GetTransform().Parent.GetTransform().RemoveChild(entity);

		for (auto& child : entity.GetTransform().GetChildren())
			DestroyEntity(child);

		Utils::Vector<Component*> components = entity.GetComponents();
		for (Component* comp : components)
			comp->OnComponentRemoved();

		m_Registry.DestroyEntity(entity);
	}

	Entity Scene::GetEntityWithUUID(UUID id)
	{
		for (auto entity : m_Registry)
		{
			EntityData& data = m_Registry.GetEntityData(entity);
			if (data.ID == id)
				return Entity{ entity, this };
		}
		return Entity::null;
	}

	SceneSystem* Scene::CreateSceneSystem(const Reflect::Class& systemClass)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (m_SystemsMap[systemClass.GetTypeID()] != nullptr)
		{
			CORE_WARN("can not create sceen system of type {0} system alreaty exists", systemClass.GetSname());
			return nullptr;
		}
		SceneSystem* system = (SceneSystem*)systemClass.CreateInstance();
		m_Systems.Push(system);
		m_SystemsMap[systemClass.GetTypeID()] = system;
		system->OnAttatch();

		RegenerateUpdateEvents();
		return system;
	}

	Utils::Vector<SceneSystem*> Scene::CreateSceneSystems(const Utils::Vector<const Reflect::Class*>& systemClasses)
	{
		CREATE_PROFILE_FUNCTIONI();
		Utils::Vector<SceneSystem*> systems;
		systems.Reserve(systemClasses.Count());
		for (const Reflect::Class* systemClass : systemClasses)
		{
			if (m_SystemsMap[systemClass->GetTypeID()] != nullptr)
			{
				CORE_WARN("can not create sceen system of type {0} system alreaty exists", systemClass->GetSname());
				continue;
			}

			SceneSystem* system = (SceneSystem*)systemClass->CreateInstance();
			m_Systems.Push(system);
			systems.Push(system);
			m_SystemsMap[systemClass->GetTypeID()] = system;
			system->OnAttatch();
		}

		RegenerateUpdateEvents();
		return systems;
	}

	void Scene::RemoveSceneSystem(const Reflect::Class& systemClass)
	{
		CREATE_PROFILE_FUNCTIONI();
		auto systemEntry = m_SystemsMap.find(systemClass.GetTypeID());
		SceneSystem* system = systemEntry->second;
		system->OnDetatch();

		m_SystemsMap.erase(systemEntry);
		m_Systems.RemoveRange(std::remove(m_Systems.begin(), m_Systems.end(), system), m_Systems.end());

		delete system;
	}

	void Scene::RegenerateUpdateEvents()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_UpdateEvents.Clear();
		m_SceneScript->GenerateUpdateEvents();
	}

	Ref<Scene> Scene::Create()
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<Scene> scene = CreateRef<Scene>(SceneScriptBase::GetDefultSceneScriptInstance());
		Window& window = Application::Get().GetWindow();
		scene->OnViewportResize(window.GetWidth(), window.GetHeight());
		scene->GetSceneScript()->OnSceneLoad();
		scene->RegenerateUpdateEvents();
		return scene;
	}

	Ref<Scene> Scene::Create(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<Scene> scene = CreateRef<Scene>(SceneScriptBase::GetDefultSceneScriptInstance());
		SceneSerializer serializer(scene);
		serializer.Deserialize(file.string());
		Window& window = Application::Get().GetWindow();
		scene->OnViewportResize(window.GetWidth(), window.GetHeight());
		scene->GetSceneScript()->OnSceneLoad();
		scene->RegenerateUpdateEvents();
		return scene;
	}

	Ref<Scene> Scene::CreateDefault()
	{
		CREATE_PROFILE_FUNCTIONI();
		uint64 defaultSceneID = Application::Get().GetProject().GetDefaultSceneID();
		if (defaultSceneID == 0) // create empty scene if no default scene is given
			return Create();
		fs::path defaultSceneFile = Application::Get().GetAssetManager().GetAssetPath(defaultSceneID);
		return Create(defaultSceneFile);
	}








	SceneUpdateEvent::SceneUpdateEvent(const std::string& funcName) :
		m_FuncName(funcName)
	{}

	void SceneUpdateEvent::Setup(Scene* scene)
	{
		UpdateEvent::Setup(scene);

		const std::vector<const Reflect::Class*> componentClasses = Reflect::Registry::GetRegistry()->GetGroup("Component");
		for (const Reflect::Class* componentClass : componentClasses)
		{
			if (componentClass->HasFunction(m_FuncName))
			{
				m_Pools.Push(scene->GetRegistry().GetOrCreateCompnentPool(*componentClass));
				m_Funcs.Push(&componentClass->GetFunction(m_FuncName));
			}
		}
	}

	void SceneUpdateEvent::Update()
	{
		CREATE_PROFILE_SCOPEI_DYNAMIC(m_FuncName);
		for (uint32 i = 0; i < m_Pools.Count(); i++)
		{
			ComponentPool* pool = m_Pools[i];
			const Reflect::Function* func = m_Funcs[i];
			pool->EachEntity([func](void* component) {
				func->Invoke(component, {});
			});
		}
	}

}
