#include "Components.h"

namespace Engine
{

	std::vector<AABB> Component::GetVolumes()
	{
		if (m_DirtyVolumes)
		{
			m_Volumes = GenVolumes();
			m_DirtyVolumes = false;
		}
		return m_Volumes;
	}

}

