#pragma once
#include "Engine/Core/Core.h"
#include "Renderer/Model.h"

namespace Game
{
#if defined(EDITOR)
	bool DrawModelControl(const std::string& label, Engine::Ref<Model>& mesh);
#endif
}
