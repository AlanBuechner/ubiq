#pragma once

#include <Engine.h>
#include <glm/glm.hpp>

namespace Engine
{

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:

		Ref<OrthographicCameraControler> m_Camera;

		Ref<Texture2D> m_LogoTexture;
		Ref<SubTexture2D> m_Texture;

		glm::vec2 m_ViewPortSize;
		Ref<FrameBuffer> m_FrameBuffer;

	};
}