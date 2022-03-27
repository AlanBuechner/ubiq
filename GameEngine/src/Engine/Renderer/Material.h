#pragma once
#include "Engine\Core\core.h"
#include "Shader.h"
#include "Texture.h"

namespace Engine
{
	class Material
	{
	public:
		Ref<Shader> shader;
		Ref<Texture2D> diffuse;
		Ref<Texture2D> speculur;
		Ref<Texture2D> normal;
	};
}
