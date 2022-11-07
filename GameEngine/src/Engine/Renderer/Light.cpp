#include "pch.h"
#include "Light.h"

namespace Engine
{

	void DirectionalLight::SetAngles(Math::Vector2 rot)
	{
		m_Angles = rot;
		rot = { Math::Radians(rot.x), Math::Radians(rot.y) };
		Math::Vector3 dir = {
			Math::Cos(rot.x) * Math::Cos(rot.y),
			Math::Sin(rot.y),
			Math::Sin(rot.x) * Math::Cos(rot.y),
		};
		SetDirection(dir);
	}

}

