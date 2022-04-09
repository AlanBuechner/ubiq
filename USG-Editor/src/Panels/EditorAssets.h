#pragma once
#include <Engine.h>

namespace Engine
{
	class EditorAssets
	{
	public:
		static void Init();

		static Ref<Texture2D> s_DefaultFileIcon;
		static Ref<Texture2D> s_ShaderFileIcon;
		static Ref<Texture2D> s_SceneFileIcon;
		static Ref<Texture2D> s_ImageFileIcon;
		static Ref<Texture2D> s_FolderIcon;
		static Ref<Texture2D> s_BackIcon;
		
		static Ref<Texture2D> s_NoTextureIcon;

	};
}
