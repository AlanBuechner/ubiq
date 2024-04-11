#pragma once
#include <Engine.h>

namespace Engine
{
	class EditorAssets
	{
	public:
		static void Init();
		static void Destroy();

		// textures
		static Ref<Texture2D> s_DefaultFileIcon;
		static Ref<Texture2D> s_ShaderFileIcon;
		static Ref<Texture2D> s_SceneFileIcon;
		static Ref<Texture2D> s_ImageFileIcon;
		static Ref<Texture2D> s_FolderIcon;
		static Ref<Texture2D> s_BackIcon;
		
		static Ref<Texture2D> s_NoTextureIcon;

		static Ref<Texture2D> s_PlayButton;
		static Ref<Texture2D> s_StopButton;

		static Ref<Texture2D> s_CameraIcon;

		// shaders

	};
}
