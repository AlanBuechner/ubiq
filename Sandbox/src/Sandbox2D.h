#pragma once

#include <Engine.h>
#include <Engine/Math/Math.h>

class Sandbox2DLayer : public Engine::Layer
{
public:
	Sandbox2DLayer();
	~Sandbox2DLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Engine::Event& event) override;
private:

	Engine::Ref<Engine::Texture2D> m_LogoTexture;
	Engine::Ref<Engine::SubTexture2D> m_Texture;

	Engine::Ref<Engine::CommandList> m_CommandList;
	Engine::Ref<Engine::FrameBuffer> m_Frame;

	Math::Vector2 m_ViewPortSize{ 0,0 };

	Engine::Camera m_Camera;
	Math::Mat4 m_CameraTransform;

};
