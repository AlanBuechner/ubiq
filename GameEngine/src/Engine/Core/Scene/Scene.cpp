#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/EditorCamera.h"

#include "glm/glm.hpp"


namespace Engine
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::OnUpdateEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		// render sprites
		auto sprite = entt::get<SpriteRendererComponent>;
		auto group = m_Registry.group<TransformComponent>(sprite);
		for (auto entity : group)
		{
			auto [transform, mesh] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.GetTransform(), mesh.Color);
		}
		Renderer2D::EndScene();
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

		// get main camera
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
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

				Renderer2D::DrawQuad(transform.GetTransform(), mesh.Color);
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
		Entity entity{ m_Registry.create(), this };
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
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
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
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

}