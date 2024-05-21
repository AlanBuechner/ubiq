#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "Engine/Core/UUID.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Core/Scene/TransformComponent.h"

#include "glm/glm.hpp"

namespace Engine
{
	Scene::Scene()
	{
		m_SceneRenderer = SceneRenderer::Create();
		OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

	Scene::~Scene()
	{
		m_SceneRenderer = nullptr;
	}

	void Scene::OnUpdate(Ref<Camera> camera)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_CameraChanged = m_SceneRenderer->GetMainCamera() != camera;
		if(m_CameraChanged)
			m_SceneRenderer->SetMainCamera(camera);

		// updating
		{
			// update transforms
			CREATE_PROFILE_SCOPEI("Update Transforms");
			auto TransformComponentView = m_Registry.View<TransformComponent>();
			for (TransformComponent& tc : TransformComponentView)
				tc.UpdateHierarchyGlobalTransform();
		}

		{
			CREATE_PROFILE_SCOPEI("Prepare components for rendering");
			//update invalid components
			m_Registry.EachPool([&](ComponentPool* pool) {
				pool->EachEntity([&](void* component) {
					((Component*)component)->OnPreRender();
				});
			});
		}
		
		// render 3d models
		m_SceneRenderer->UpdateBuffers();
	}

	void Scene::OnViewportResize(uint32 width, uint32 height)
	{
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

		std::vector<Component*> components = entity.GetComponents();
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
}
