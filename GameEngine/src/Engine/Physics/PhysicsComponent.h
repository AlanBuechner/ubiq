#pragma once
#include "Engine/Math/Math.h"

namespace Engine
{
	struct Rigidbody2DComponent
	{
		enum class BodyType {
			Static = 0,
			Dynamic = 1,
			Kinematic = 2
		};
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent& other) = default;

		void AddForce(Math::Vector2 force);
		void AddForce(Math::Vector2 force, Math::Vector2 point);

	};

	struct BoxCollider2DComponent
	{
		Math::Vector2 Offset = { 0.0f, 0.0f };
		Math::Vector2 Size = { 0.5f, 0.5f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other) = default;
	};

	struct CircleColliderComponent
	{
		Math::Vector2 Offset = { 0.0f, 0.0f };
		float Size = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleColliderComponent() = default;
		CircleColliderComponent(const CircleColliderComponent& other) = default;
	};
}
