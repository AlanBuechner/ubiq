#include "Components.h"

namespace Engine
{

	Utils::Vector<AABB> Component::GetVolumes()
	{
		if (m_DirtyVolumes)
		{
			m_Volumes = GenVolumes();
			m_DirtyVolumes = false;
		}
		return m_Volumes;
	}

}

