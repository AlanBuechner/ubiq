#include "EditorAssets.h"
#include "Resource.h"
#include "Engine/AssetManager/AssetManager.h"

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
		fs::path p = "";
		s_DefaultFileIcon =		Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(DEFULTFILEICON);
		s_ShaderFileIcon =		Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(SHADERFILEICON);
		s_SceneFileIcon =		Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(SCENEFILEICON);
		s_ImageFileIcon =		Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(IMAGEFILEICON);
		s_FolderIcon =			Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(FOLDERICON);
		s_BackIcon =			Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(BACKICON);

		s_NoTextureIcon =		Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(NOTEXTUREICON);

		s_PlayButton =			Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(PLAYBUTTON);
		s_StopButton =			Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(STOPBUTTON);

		s_CameraIcon =			Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Texture2D>(CAMERAICON);

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
