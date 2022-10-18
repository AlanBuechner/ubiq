#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h" 
#include "Entity.h"
#include "Engine/Core/UUID.h"

#include "Engine/Physics/Physics2D.h"
#include "Engine/Physics/PhysicsComponent.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/SceneRendererComponents.h"

#include "glm/glm.hpp"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

namespace Engine
{
	Scene::Scene()
	{
		m_CameraIcon = Texture2D::Create("Resources/CameraIcon.png");
		m_SceneRenderer = SceneRenderer::Create();
	}

	Scene::~Scene()
	{
		m_Registry.clear();
		m_SceneRenderer = nullptr;
	}

	void Scene::OnRuntimeStart()
	{
		Physics2D::OnPhysicsStart(this);
	}

	void Scene::OnRuntimeStop()
	{
		Physics2D::OnPhysicsStop();
	}

	void Scene::OnUpdateEditor(const EditorCamera& camera)
	{
		// update transforms
		m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, this };
			entity.GetTransform().UpdateHierarchyGlobalTransform();
		});

		// render sprites
		{
			Renderer2D::BeginScene(camera);

			// render sprites
			auto sprite = entt::get<SpriteRendererComponent>;
			auto spriteGroup = m_Registry.group<TransformComponent>(sprite);
			for (auto entity : spriteGroup)
			{
				auto [transform, sprite] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetGlobalTransform(), sprite, (int)entity);
			}

			// render icons
			auto cameraView = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : cameraView)
			{
				auto [cam, transform] = cameraView.get<CameraComponent, TransformComponent>(entity);
				TransformComponent t = transform;
				t.SetRotation({ -camera.GetPitch(), -camera.GetYaw(), 0 });
				t.SetScale({ 1,1,1 });

				Renderer2D::DrawQuad(t.GetTransform(), m_CameraIcon, (int)entity);

			}

			Renderer2D::EndScene();
		}

		// render 3d models
		{
			m_SceneRenderer->BeginScene(camera);
			m_SceneRenderer->Invalidate();
		}
	}

	void Scene::OnUpdateRuntime()
	{

		// update scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) 
		{
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = {entity, this};
				nsc.Instance->OnCreate();
			}
			nsc.Instance->OnUpdate();
		});

		// Physics
		Physics2D::OnPysicsUpdate();

		// update transforms
		m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, this };
			entity.GetTransform().UpdateHierarchyGlobalTransform();
		});

		// get main camera
		Camera* mainCamera = nullptr;
		Math::Mat4 cameraTransform;
		{
			auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetGlobalTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			// render sprites
			auto sprite = entt::get<SpriteRendererComponent>;
			auto group = m_Registry.group<TransformComponent>(sprite);
			for (auto entity : group)
			{
				auto [transform, mesh] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetGlobalTransform(), mesh, -1);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32 width, uint32 height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(const UUID uuid, const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<EntityDataComponent>(name.empty() ? "Entity" : name, uuid);
		auto& tc = entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.GetTransform().Parent)
			entity.GetTransform().Parent.GetTransform().RemoveChild(entity);

		for (auto& child : entity.GetTransform().GetChildren())
			DestroyEntity(child);


		m_Registry.destroy(entity);
	}

	Entity Scene::GetEntityWithUUID(UUID id)
	{
		auto view = m_Registry.view<EntityDataComponent>();
		for (auto entity : view) {
			auto idcomp = view.get<EntityDataComponent>(entity);
			if (idcomp.GetID() == id)
				return Entity{ entity, this };
		}
		return Entity::null;
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return Entity::null;
	}

	template<typename T>
	static void CopyComponent(entt::registry& dest, entt::registry& src, const std::unordered_map<UUID, entt::entity>& map)
	{
		auto view = src.view<T>();
		for (auto srcEntity : view)
		{
			entt::entity destEntity = map.at(src.get<EntityDataComponent>(srcEntity).GetID());
			auto& srcComponent = src.get<T>(srcEntity);
			dest.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> scene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto& srcSceneRegistry = scene->m_Registry;
		auto& destSceneRegisry = newScene->m_Registry;
		auto idView = srcSceneRegistry.view<EntityDataComponent>();
		for (auto e : idView)
		{
			auto uuid = srcSceneRegistry.get<EntityDataComponent>(e).GetID();
			const auto& name = srcSceneRegistry.get<EntityDataComponent>(e).Name;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = newEntity;
		}

		CopyComponent<TransformComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(destSceneRegisry, srcSceneRegistry, enttMap);
		CopyComponent<CircleColliderComponent>(destSceneRegisry, srcSceneRegistry, enttMap);

		return newScene;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		//static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{

	}
	
	template<>
	void Scene::OnComponentAdded<CircleColliderComponent>(Entity entity, CircleColliderComponent& component)
	{

	}

}
