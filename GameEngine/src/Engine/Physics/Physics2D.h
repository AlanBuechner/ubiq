#pragma once

#include "Engine/Core/Scene/Entity.h"
#include "glm/glm.hpp"

class b2World;

namespace Engine
{
	class Scene;
}

namespace Engine
{

	struct RayCastHit
	{
		Math::Vector2 point;
		Math::Vector2 normal;
		float distanceSqr;
		Entity entity;
	};

	class Physics2D
	{
	public:
		static void OnPhysicsStart(Scene* scene);
		static void OnPhysicsStop();
		static void OnPysicsUpdate();

		static Math::Vector2 GetGravity();
		static void SetGravity(float g);
		static void SetGravity(Math::Vector2 g);

		static bool RayCast(Math::Vector2 point, Math::Vector2 dir, float dist, RayCastHit& outHit);
		static bool RayCast(Math::Vector2 p1, Math::Vector2 p2, RayCastHit& outHit);

	private:
		static b2World* s_World;

		static Scene* s_Scene;



	};

}
