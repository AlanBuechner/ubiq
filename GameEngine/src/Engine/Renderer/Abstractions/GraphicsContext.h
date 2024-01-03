#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual Ref<ResourceManager> GetResourceManager() = 0;

		static Ref<GraphicsContext> Create();
	};
}
