#include "EditorAssets.h"
#include "Resource.h"

// textures
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_DefaultFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_ShaderFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_SceneFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_ImageFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_FolderIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_BackIcon;

Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_NoTextureIcon;

Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_PlayButton;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_StopButton;

Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_CameraIcon;

// shaders


namespace Engine
{
	void EditorAssets::Init()
	{
		CORE_INFO("Loading Editor Assets");
		// Textures
		s_DefaultFileIcon =		Texture2D::CreateFromEmbeded(DEFULTFILEICON);
		s_ShaderFileIcon =		Texture2D::CreateFromEmbeded(SHADERFILEICON);
		s_SceneFileIcon =		Texture2D::CreateFromEmbeded(SCENEFILEICON);
		s_ImageFileIcon =		Texture2D::CreateFromEmbeded(IMAGEFILEICON);
		s_FolderIcon =			Texture2D::CreateFromEmbeded(FOLDERICON);
		s_BackIcon =			Texture2D::CreateFromEmbeded(BACKICON);
												 
		s_NoTextureIcon =		Texture2D::CreateFromEmbeded(NOTEXTUREICON);
												 
		s_PlayButton =			Texture2D::CreateFromEmbeded(PLAYBUTTON);
		s_StopButton =			Texture2D::CreateFromEmbeded(STOPBUTTON);
												 
		s_CameraIcon =			Texture2D::CreateFromEmbeded(CAMERAICON);

		// shaders
	}

	void EditorAssets::Destroy()
	{
		// textures
		s_DefaultFileIcon.reset();
		s_ShaderFileIcon.reset();
		s_SceneFileIcon.reset();
		s_ImageFileIcon.reset();
		s_FolderIcon.reset();
		s_BackIcon.reset();

		s_NoTextureIcon.reset();

		s_PlayButton.reset();
		s_StopButton.reset();

		s_CameraIcon.reset();

		// shaders
	}

}
