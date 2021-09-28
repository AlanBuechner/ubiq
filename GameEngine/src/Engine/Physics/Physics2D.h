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
		glm::vec2 point;
		glm::vec2 normal;
		float distanceSqr;
		Entity entity;
	};

	class Physics2D
	{
	public:
		static void OnPhysicsStart(Scene* scene);
		static void OnPhysicsStop();
		static void OnPysicsUpdate();

		static glm::vec2 GetGravity();
		static void SetGravity(float g);
		static void SetGravity(glm::vec2 g);

		static bool RayCast(glm::vec2 point, glm::vec2 dir, float dist, RayCastHit& outHit);
		static bool RayCast(glm::vec2 p1, glm::vec2 p2, RayCastHit& outHit);

	private:
		static b2World* s_World;

		static Scene* s_Scene;



	};

}
