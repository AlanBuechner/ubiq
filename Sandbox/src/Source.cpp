#include <Engine.h>

class EditerCamera : public Engine::PerspectiveCamera
{
public:
	EditerCamera()
		: Super(60.0f, 16.0f/9.0f)
	{}

	InputControler* Input;
	float Speed = 0.15f;
	glm::vec2 prevMousePos;
	glm::vec3 MoveDir = { 0.0f, 0.0f, 0.0f };

	void SetPlayerInput(Engine::InputControlerManeger* maneger)
	{
		Input = new InputControler(maneger);

		Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&EditerCamera::StartMove, glm::vec3({  0.0f, 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&EditerCamera::StartMove, glm::vec3({  0.0f, 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&EditerCamera::StartMove, glm::vec3({ -1.0f, 0.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&EditerCamera::StartMove, glm::vec3({  1.0f, 0.0f,  0.0f })));

		Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&EditerCamera::EndMove, glm::vec3({  0.0f, 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&EditerCamera::EndMove, glm::vec3({  0.0f, 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&EditerCamera::EndMove, glm::vec3({ -1.0f, 0.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&EditerCamera::EndMove, glm::vec3({  1.0f, 0.0f,  0.0f })));
	}

	void Update()
	{
		Translate(MoveDir * Speed);
	}

private:
	void StartMove(const glm::vec3& movedir)
	{
		MoveDir += movedir;
	}

	void EndMove(const glm::vec3& movedir)
	{
		MoveDir -= movedir;
	}
};

class ExampleLayer : public Engine::Layer
{
public:
	InputControler* input;

	std::shared_ptr<Engine::Shader> m_Shader;
	std::shared_ptr<Engine::VertexArray> m_VertexArray;
	std::shared_ptr<Engine::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Engine::IndexBuffer> m_IndexBuffer;

	EditerCamera m_Camera;

	bool set = false;
	ExampleLayer()
		: Layer("Example")
	{
		input = new InputControler(m_InputManeger);

		m_Camera.SetPlayerInput(m_InputManeger);


		m_VertexArray.reset(Engine::VertexArray::Create());

		float vertices[4 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
		};

		m_VertexBuffer.reset(Engine::VertexBuffer::Create(vertices, sizeof(vertices)));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float4, "a_Color"}
		};

		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 0, 1, 2,  0, 3, 2 };

		m_IndexBuffer.reset(Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		Engine::Shader::ShaderSorce src;
		src << Engine::Shader::LoadShader("C:\\Users\\Alan\\source\\repos\\GameEngine\\shader.hlsl");

		m_Shader.reset(Engine::Shader::Create(src.vertexShader, src.pixleShader));

	}

	void OnUpdate() override
	{
		m_Camera.Update();

		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_Camera);

		Engine::Renderer::Submit(m_VertexArray, m_Shader);

		Engine::Renderer::EndScene();

		//Engine::Renderer::Flush();
	}

	void OnEvent(Engine::Event& event) override
	{
		Super::OnEvent(event);
		if (event.GetEventType() == Engine::EventType::KeyPressed)
		{
			Engine::KeyPressedEvent& e = (Engine::KeyPressedEvent&)event;
		}
	}
};

class Sandbox : public Engine::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox();
}