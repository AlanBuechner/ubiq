#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h" 
#include "Entity.h"
#include "Engine/Core/UUID.h"

#include "Engine/Physics/Physics2D.h"
#include "Engine/Physics/PhysicsComponent.h"

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"
#include "Engine/Renderer/EditorCamera.h"

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
	}

	Scene::~Scene()
	{
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

		{
			Renderer2D::BeginScene(camera);

			// render sprites
			auto sprite = entt::get<SpriteRendererComponent>;
			auto spriteGroup = m_Registry.group<TransformComponent>(sprite);
			for (auto entity : spriteGroup)
			{
				auto [transform, sprite] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}

			// render icons
			auto cameraView = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : cameraView)
			{
				auto [cam, transform] = cameraView.get<CameraComponent, TransformComponent>(entity);
				TransformComponent t = transform;
				t.Rotation = { -camera.GetPitch(), -camera.GetYaw(), 0 };
				t.Scale = { 1,1,1 };

				Renderer2D::DrawQuad(t.GetTransform(), m_CameraIcon, (int)entity);

			}

			Renderer2D::EndScene();
		}

		{
			Renderer::BeginScene(camera);
			Renderer::SubmitPointLight({ {1,1,0}, {1,1,1}, 5.0f, 1, 1, 0.5f });

			auto view = m_Registry.view<MeshRendererComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto [mesh, transform] = view.get<MeshRendererComponent, TransformComponent>(entity);

				Renderer::Submit(mesh.vao, mesh.mat, transform.GetTransform());
			}

			Renderer::EndScene();
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
					cameraTransform = transform.GetTransform();
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

				Renderer2D::DrawSprite(transform.GetTransform(), mesh, -1);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
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
		entity.AddComponent<IDComponent>(uuid);
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	static void CopyComponent(entt::registry& dest, entt::registry& src, const std::unordered_map<UUID, entt::entity>& map)
	{
		auto view = src.view<T>();
		for (auto srcEntity : view)
		{
			entt::entity destEntity = map.at(src.get<IDComponent>(srcEntity).ID);
			auto& srcComponent = src.get<T>(srcEntity);
			dest.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> scene)
	{
		Ref<Scene> newScene = std::make_shared<Scene>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto& srcSceneRegistry = scene->m_Registry;
		auto& destSceneRegisry = newScene->m_Registry;
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			auto uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
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
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{

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
