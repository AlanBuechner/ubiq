#define _XKEYCHECK_H
#include <pch.h>
#include <stdarg.h>
#include "Reflection.h"
#define private public
#include "C:/GameDev/GameEngine/ubiq/EditorModule/src/Editor/Panels/ContentBrowserPanel.h"
#include "src\Editor/Panels/GridGizmosPanel.h"
#include "C:/GameDev/GameEngine/ubiq/EditorModule/src/Editor/Panels/SceneHierarchyPanel.h"
static Reflect::Registry::Add<Editor::GridGizmosPanel> ClassGridGizmosPanel("GridGizmosPanel", "Editor::GridGizmosPanel", "EditorPanel",{"reflect-class"}, {{"GROUP","EditorPanel"}}, {}, {});
static Reflect::Registry::Add<Editor::ContentBrowserPanel> ClassContentBrowserPanel("ContentBrowserPanel", "Editor::ContentBrowserPanel", "",{"reflect-class","EditorPanel"}, {{"GROUP",""}}, {}, {});
static Reflect::Registry::Add<Editor::SceneHierarchyPanel> ClassSceneHierarchyPanel("SceneHierarchyPanel", "Editor::SceneHierarchyPanel", "EditorPanel",{"reflect-class"}, {{"GROUP","EditorPanel"}}, {}, {});
void DeadLinkEditorModule() {}
