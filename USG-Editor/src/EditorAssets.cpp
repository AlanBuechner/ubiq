#include "EditorAssets.h"

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
		s_DefaultFileIcon = Texture2D::Create("Resources/Textures/DefaultFileIcon.png");
		s_ShaderFileIcon = Texture2D::Create("Resources/Textures/ShaderFileIcon.png");
		s_SceneFileIcon = Texture2D::Create("Resources/Textures/SceneFileIcon.png");
		s_ImageFileIcon = Texture2D::Create("Resources/Textures/ImageFileIcon.png");
		s_FolderIcon = Texture2D::Create("Resources/Textures/FolderIcon.png");
		s_BackIcon = Texture2D::Create("Resources/Textures/BackIcon.png");

		s_NoTextureIcon = Texture2D::Create("Resources/Textures/NoTextureIcon.png");

		s_PlayButton = Texture2D::Create("Resources/Textures/PlayButton.png");
		s_StopButton = Texture2D::Create("Resources/Textures/StopButton.png");

		s_CameraIcon = Texture2D::Create("Resources/Textures/CameraIcon.png");

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
