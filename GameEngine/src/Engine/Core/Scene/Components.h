#pragma once
#include <Engine/Math/Math.h>

#include "Engine/Core/UUID.h"
#include "Entity.h"

#include "Reflection.h"

namespace Engine
{
	class Component : public Reflect::Reflected
	{
	public:
		Entity Owner;

		virtual void OnComponentAdded() {}
		virtual void OnComponentRemoved() {}

		virtual void OnTransformChange(const Math::Mat4& transform) {}
		virtual void OnViewportResize(uint32 width, uint32 height) {}
	};

}
