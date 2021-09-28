#include "pch.h"
#include "PhysicsComponent.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"

namespace Engine
{
	void Rigidbody2DComponent::AddForce(glm::vec2 f)
	{
		((b2Body*)RuntimeBody)->ApplyForceToCenter({f.x, f.y}, true);
	}

	void Rigidbody2DComponent::AddForce(glm::vec2 f, glm::vec2 p)
	{
		((b2Body*)RuntimeBody)->ApplyForce({ f.x, f.y }, {p.x, p.y}, true);
	}

}
