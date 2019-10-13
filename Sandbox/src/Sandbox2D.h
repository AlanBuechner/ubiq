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

	Engine::ShaderLibrary m_ShaderLib;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer;
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer;
	Engine::Ref<Engine::Texture2D> m_Texture;

	Engine::Ref<Engine::OrthographicCameraControler> m_Camera;

};