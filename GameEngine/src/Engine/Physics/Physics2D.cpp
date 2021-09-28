#include "pch.h"
#include "Physics2D.h"
#include "PhysicsComponent.h"

#include "Engine/Core/Scene/Entity.h"
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/Components.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

b2World* Engine::Physics2D::s_World = nullptr;
Engine::Scene* Engine::Physics2D::s_Scene = nullptr;


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
	void Physics2D::OnPhysicsStart(Scene* scene)
	{
		s_Scene = scene;

		s_World = new b2World({ 0.0f, -9.81f });

		auto view = s_Scene->m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, s_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = GetB2BodyType(rb2d.Type);
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = s_World->CreateBody(&bodyDef);
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
				fixtureDef.userData.pointer = (uintptr_t)e;

				bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}
		}

	}

	void Physics2D::OnPhysicsStop()
	{
		delete s_World;
		s_World = nullptr;
	}

	void Physics2D::OnPysicsUpdate()
	{
		auto view = s_Scene->m_Registry.view<Rigidbody2DComponent>();

		for (auto e : view)
		{
			Entity entity = { e, s_Scene };

			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ transform.Position.x, transform.Position.y }, transform.Rotation.z);

		}

		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		s_World->Step(Time::GetDeltaTime(), velocityIterations, positionIterations);

		for (auto e : view)
		{
			Entity entity = { e, s_Scene };

			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			transform.Position.x = position.x;
			transform.Position.y = position.y;
			transform.Rotation.z = body->GetAngle();
		}
	}

	glm::vec2 Physics2D::GetGravity()
	{
		b2Vec2 g = s_World->GetGravity();
		return {g.x, g.y};
	}

	void Physics2D::SetGravity(float g)
	{
		s_World->SetGravity({0.0f, g});
	}

	void Physics2D::SetGravity(glm::vec2 g)
	{
		s_World->SetGravity({ g.x, g.y });
	}

	bool Physics2D::RayCast(glm::vec2 point, glm::vec2 dir, float dist, RayCastHit& outHit)
	{
		return RayCast(point, point + glm::normalize(dir)*dist, outHit);
	}

	bool Physics2D::RayCast(glm::vec2 p1, glm::vec2 p2, RayCastHit& outHit)
	{

		class Callback : public b2RayCastCallback
		{
		public:
			Callback(glm::vec2& p1, glm::vec2& p2, RayCastHit& outHit) :
				p1(p1), p2(p2), outHit(outHit)
			{}

			glm::vec2 &p1, &p2;
			RayCastHit& outHit;

			bool hit = false;

			virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
				const b2Vec2& normal, float fraction) override
			{
				outHit.point = { point.x, point.y };
				outHit.normal = { normal.x, normal.y };
				glm::vec2 dir = p1 - outHit.point;
				outHit.distanceSqr = dir.x * dir.x + dir.y * dir.y;
				outHit.entity = { (entt::entity)fixture->GetUserData().pointer, s_Scene };
				hit = true;
				return 0.0f;
			}
		} callback(p1,p2,outHit);

		s_World->RayCast(&callback, { p1.x, p1.y }, {p2.x, p2.y});

		return callback.hit;
	}

}
