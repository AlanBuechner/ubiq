#include "EditorAssets.h"

Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_DefaultFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_ShaderFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_SceneFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_ImageFileIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_FolderIcon;
Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_BackIcon;

Engine::Ref<Engine::Texture2D> Engine::EditorAssets::s_NoTextureIcon;

void Engine::EditorAssets::Init()
{
	s_DefaultFileIcon = Texture2D::Create("Resources/DefaultFileIcon.png");
	s_ShaderFileIcon = Texture2D::Create("Resources/ShaderFileIcon.png");
	s_SceneFileIcon = Texture2D::Create("Resources/SceneFileIcon.png");
	s_ImageFileIcon = Texture2D::Create("Resources/ImageFileIcon.png");
	s_FolderIcon = Texture2D::Create("Resources/FolderIcon.png");
	s_BackIcon = Texture2D::Create("Resources/BackIcon.png");

	s_NoTextureIcon = Texture2D::Create("Resources/NoTextureIcon.png");
}
