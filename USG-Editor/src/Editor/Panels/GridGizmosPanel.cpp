#include "GridGizmosPanel.h"
#include "Editor/EditorLayer.h"

namespace Editor
{

	GridGizmosPanel::GridGizmosPanel()
	{
		m_GridMesh.m_Vertices.Reserve((size_t)(m_GridLines + 1) * 3);
		m_GridMesh.m_Indices.Reserve((size_t)(m_GridLines + 1) * 4);
		for (uint32 i = 0; i <= m_GridLines; i++)
		{
			float posz = (m_GridLineOffset * i) - m_GridExtent;
			m_GridMesh.m_Vertices.Push({ { -m_GridExtent	,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });
			m_GridMesh.m_Vertices.Push({ { 0				,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, m_GridColor.w - (m_GridColor.w * (abs(posz) / m_GridExtent)) } });
			m_GridMesh.m_Vertices.Push({ { m_GridExtent	,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });

			m_GridMesh.m_Indices.Push((i * 3) + 0);
			m_GridMesh.m_Indices.Push((i * 3) + 1);
			m_GridMesh.m_Indices.Push((i * 3) + 1);
			m_GridMesh.m_Indices.Push((i * 3) + 2);
		}
	}

	void GridGizmosPanel::OnDrawGizmos()
	{
		Math::Vector3 camPos = EditorLayer::Get()->GetEditorCamera()->GetPosition();

		Math::Mat4 matz = glm::translate(Math::Mat4(1.0f), { camPos.x, 0, camPos.z - fmod(camPos.z, m_GridLineOffset) });
		Math::Mat4 matx = glm::translate(Math::Mat4(1.0f), { camPos.x - fmod(camPos.x, m_GridLineOffset), 0, camPos.z })
			* glm::rotate(Math::Mat4(1.0f), glm::radians(90.0f), { 0,1,0 });

		Engine::DebugRenderer::DrawLineMesh(m_GridMesh, matz);
		Engine::DebugRenderer::DrawLineMesh(m_GridMesh, matx);
	}

}

