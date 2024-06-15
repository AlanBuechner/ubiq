#include "EditorAssets.h"
#include "Resource.h"

// textures
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_DefaultFileIcon;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_ShaderFileIcon;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_SceneFileIcon;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_ImageFileIcon;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_FolderIcon;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_BackIcon;

Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_NoTextureIcon;

Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_PlayButton;
Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_StopButton;

Engine::Ref<Engine::Texture2D> Editor::EditorAssets::s_CameraIcon;

// shaders


namespace Editor
{
	void EditorAssets::Init()
	{
		CORE_INFO("Loading Editor Assets");
		// Textures
		s_DefaultFileIcon =		Engine::Texture2D::CreateFromEmbeded(DEFULTFILEICON);
		s_ShaderFileIcon =		Engine::Texture2D::CreateFromEmbeded(SHADERFILEICON);
		s_SceneFileIcon =		Engine::Texture2D::CreateFromEmbeded(SCENEFILEICON);
		s_ImageFileIcon =		Engine::Texture2D::CreateFromEmbeded(IMAGEFILEICON);
		s_FolderIcon =			Engine::Texture2D::CreateFromEmbeded(FOLDERICON);
		s_BackIcon =			Engine::Texture2D::CreateFromEmbeded(BACKICON);

		s_NoTextureIcon =		Engine::Texture2D::CreateFromEmbeded(NOTEXTUREICON);

		s_PlayButton =			Engine::Texture2D::CreateFromEmbeded(PLAYBUTTON);
		s_StopButton =			Engine::Texture2D::CreateFromEmbeded(STOPBUTTON);

		s_CameraIcon =			Engine::Texture2D::CreateFromEmbeded(CAMERAICON);

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
