#pragma once
#include <Engine/Math/Math.h>

#include "Engine/Physics/AABB.h"
#include "Engine/Core/UUID.h"
#include "Entity.h"

#include "Reflection.h"
#include "Engine/Core/ObjectDescription/ObjectDescription.h"

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

		Utils::Vector<AABB> GetVolumes();

	protected:
		void DirtyVolume() { m_DirtyVolumes = true; }
		virtual Utils::Vector<AABB> GenVolumes() { return Utils::Vector<AABB>(); }

	private:
		bool m_DirtyVolumes = true;
		Utils::Vector<AABB> m_Volumes;
	};

}
