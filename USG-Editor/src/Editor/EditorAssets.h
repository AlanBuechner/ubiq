#pragma once
#include <Engine.h>

namespace Editor
{
	class EditorAssets
	{
	public:
		static void Init();
		static void Destroy();

		// textures
		static Engine::Ref<Engine::Texture2D> s_DefaultFileIcon;
		static Engine::Ref<Engine::Texture2D> s_ShaderFileIcon;
		static Engine::Ref<Engine::Texture2D> s_SceneFileIcon;
		static Engine::Ref<Engine::Texture2D> s_ImageFileIcon;
		static Engine::Ref<Engine::Texture2D> s_FolderIcon;
		static Engine::Ref<Engine::Texture2D> s_BackIcon;

		static Engine::Ref<Engine::Texture2D> s_NoTextureIcon;

		static Engine::Ref<Engine::Texture2D> s_PlayButton;
		static Engine::Ref<Engine::Texture2D> s_StopButton;

		static Engine::Ref<Engine::Texture2D> s_CameraIcon;

		// shaders

	};
}
