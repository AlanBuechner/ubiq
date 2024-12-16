#pragma once
#include <Engine/Math/Math.h>

#include "Engine/Physics/AABB.h"
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

		std::vector<AABB> GetVolumes();

	protected:
		void DirtyVolume() { m_DirtyVolumes = true; }
		virtual std::vector<AABB> GenVolumes() { return std::vector<AABB>(); }

	private:
		bool m_DirtyVolumes = true;
		std::vector<AABB> m_Volumes;
	};

}
