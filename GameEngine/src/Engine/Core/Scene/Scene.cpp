#include "pch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h" 
#include "Entity.h"
#include "Engine/Core/UUID.h"

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"
#include "Engine/Renderer/EditorCamera.h"

#include "glm/glm.hpp"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

b2BodyType GetB2BodyType(Engine::Rigidbody2DComponent::BodyType type)
{
	switch (type)
	{
	case Engine::Rigidbody2DComponent::BodyType::Static:	return b2BodyType::b2_staticBody;
	case Engine::Rigidbody2DComponent::BodyType::Dynamic:	return b2BodyType::b2_dynamicBody;
	case Engine::Rigidbody2DComponent::BodyType::Kinematic:	return b2BodyType::b2_kinematicBody;
	}

	CORE_ASSERT(false, "Unknown body type");
	return b2_staticBody;
}

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

		m_World = new b2World({0.0f, -9.81f});

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = GetB2BodyType(rb2d.Type);
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_World->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape shape;
				shape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

				bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		delete m_World;
		m_World = nullptr;
	}

	void Scene::OnUpdateEditor(const EditorCamera& camera)
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
		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_World->Step(Time::GetDeltaTime(), velocityIterations, positionIterations);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };

				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				const auto& position = body->GetPosition();
				transform.Position.x = position.x;
				transform.Position.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

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

}
