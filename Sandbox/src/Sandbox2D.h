#pragma once

#include <Engine.h>
#include <glm/glm.hpp>

class Sandbox2DLayer : public Engine::Layer
{
public:
	Sandbox2DLayer();
	~Sandbox2DLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate() override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Engine::Event& event) override;
private:

	Engine::Ref<Engine::OrthographicCameraControler> m_Camera;

	Engine::Ref<Engine::Texture2D> m_LogoTexture;
	Engine::Ref<Engine::SubTexture2D> m_Texture;

};