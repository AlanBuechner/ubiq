#pragma once
#include "Engine/Core/Core.h"
#include "ResourceManager.h"

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
