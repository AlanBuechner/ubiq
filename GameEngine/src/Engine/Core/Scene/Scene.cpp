#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "Engine/Core/UUID.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/Components/SceneRendererComponents.h"
#include "Engine/Renderer/Components/StaticModelRendererComponent.h"
#include "Engine/Core/Scene/TransformComponent.h"

#include "glm/glm.hpp"

namespace Engine
{
	Scene::Scene()
	{
		m_SceneRenderer = SceneRenderer::Create();
	}

	Scene::~Scene()
	{
		m_SceneRenderer = nullptr;
	}

	void Scene::OnRuntimeStart()
	{
	}

	void Scene::OnRuntimeStop()
	{
	}

	void Scene::OnUpdateEditor(Ref<EditorCamera> camera)
	{
		CREATE_PROFILE_FUNCTIONI();
		bool newCamera = m_SceneRenderer->GetMainCamera() != camera;

		// updating
		{
			// update transforms
			CREATE_PROFILE_SCOPEI("Update Transforms");
			auto TransformComponentView = m_Registry.View<TransformComponent>();
			for (TransformComponent& tc : TransformComponentView)
				tc.UpdateHierarchyGlobalTransform();
		}

		{
			CREATE_PROFILE_SCOPEI("Update Invalid Components");
			//update invalid components
			m_Registry.EachPool([&](ComponentPool* pool) {
				pool->EachEntity([&](void* component) {
					((Component*)component)->OnInvalid();
				});
			});
		}

		// rendering
		{
			auto dirLightView = m_Registry.View<DirectionalLightComponent>();
			CREATE_PROFILE_SCOPEI("Update Directional Light Components")
			for (auto& comp : dirLightView)
			{
				CREATE_PROFILE_SCOPEI("Update Directional Light Component")
				if (newCamera)
				{
					// remove all cameras from directional light
					comp.GetDirectinalLight()->ClearCameras();
					// re add cameras to directional light
					comp.GetDirectinalLight()->AddCamera(camera);
				}
				comp.UpdateShadowMaps();
			}
		}
		
		// render 3d models
		if(newCamera)
			m_SceneRenderer->SetMainCamera(camera);
		m_SceneRenderer->UpdateBuffers();

		// render sprites
		{
			Renderer2D::BeginScene(camera);

			// render icons
			auto cameraView = m_Registry.View<CameraComponent>();
			for (auto& comp : cameraView)
			{
				TransformComponent tc = comp.Owner.GetTransform(); // create copy of transform component to billboard 
				tc.SetRotation({ -camera->GetPitch(), -camera->GetYaw(), 0 });
				tc.SetScale({ 1,1,1 });

				//Renderer2D::DrawQuad(tc.GetTransform(), m_CameraIcon, (int)(EntityType)comp.Owner);

			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateRuntime()
	{
		// update transforms
		m_Registry.EachEntity([&](EntityType entityID) {
			Entity entity{ entityID, this };
			TransformComponent& tc = entity.GetTransform();
			tc.UpdateHierarchyGlobalTransform();
		});

		// update transforms
		m_Registry.EachEntity([&](EntityType entityID) {
			Entity entity{ entityID, this };
			TransformComponent& tc = entity.GetTransform();
			tc.UpdateHierarchyGlobalTransform();
		});

		// get main camera
		Entity MainCameraEntity = GetPrimaryCameraEntity();
		Ref<Camera> mainCamera = nullptr;

		if (MainCameraEntity != Entity::null)
			mainCamera = MainCameraEntity.GetComponent<CameraComponent>()->Camera;

		if (mainCamera)
		{
			m_SceneRenderer->SetMainCamera(mainCamera);
			m_SceneRenderer->UpdateBuffers();
		}
	}

	void Scene::OnViewportResize(uint32 width, uint32 height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Registry.GetComponentPool<CameraComponent>()->EachEntity([&](void* comp) {
			CameraComponent* camComp = (CameraComponent*)comp;
			if (!camComp->FixedAspectRatio)
				camComp->Camera->SetViewportSize(width, height);
		});

		m_SceneRenderer->OnViewportResize(width, height);
	}

	Entity Scene::CreateEntity(const std::string& name)
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

	Entity Scene::GetPrimaryCameraEntity()
	{
		
		auto view = m_Registry.View<CameraComponent>();
		for (auto comp : view)
		{
			if (comp.Primary)
				return comp.Owner;
		}

		return Entity::null;
	}

	template<typename T>
	static void CopyComponent(SceneRegistry& dest, SceneRegistry& src, const std::unordered_map<UUID, EntityType>& map)
	{
		auto view = src.View<T>();
		for (auto srcComponent : view)
		{
			EntityType destEntity = map.at(src.GetEntityData(srcComponent.Owner).ID);
			//dest.AddComponent<T>(destEntity, srcComponent); // TODO add copy component
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> scene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;

		std::unordered_map<UUID, EntityType> enttMap;

		auto& srcSceneRegistry = scene->m_Registry;
		auto& destSceneRegisry = newScene->m_Registry;

		srcSceneRegistry.EachEntity([&](EntityType entity) {
			EntityData& data = srcSceneRegistry.GetEntityData(entity);
			Entity newEntity = newScene->CreateEntityWithUUID(data.ID, data.name);
			enttMap[data.ID] = newEntity;
		});

		CopyComponent<TransformComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<DirectionalLightComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<SkyboxComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<StaticModelRendererComponent>(destSceneRegisry, srcSceneRegistry, enttMap);

		return newScene;
	}

}
