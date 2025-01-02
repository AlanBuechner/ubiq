#pragma once
#include "EditorPanel.h"
#include "Engine/Renderer/DebugRenderer.h"

namespace Editor
{
	CLASS(GROUP = EditorPanel) GridGizmosPanel : public EditorPanel
	{
	public:
		REFLECTED_BODY(Editor::SceneHierarchyPanel);

	public:
		GridGizmosPanel();

		virtual void OnDrawGizmos() override;

	private:
		const Math::Vector4 m_GridColor = { 0.5f,0.5f,0.5f,1 };
		const float m_GridExtent = 40.0f;
		const uint32 m_GridLines = 80;
		const float m_GridLineOffset = m_GridExtent * 2 / m_GridLines;
		Engine::DebugMesh m_GridMesh;

	};
}
