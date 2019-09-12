#include <Engine.h>

class Player : public Engine::OrthographicCamera
{
public:
	Player()
		: Super(-1.6f, 1.6f, -0.9f, 0.9f)
	{}
	Engine::Scope<InputControler> Input;
	float Speed = 1.0f;
	glm::vec2 MoveDir = { 0.0f, 0.0f };
	glm::vec2 pos = { 0.0f, 0.0f };

	void SetPlayerInput(Engine::InputControlerManeger* maneger)
	{
		Input.reset(new InputControler(maneger));

		Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({  0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({  0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ -1.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({  1.0f,  0.0f })));

		Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({  0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({  0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ -1.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({  1.0f,  0.0f })));

		Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&Player::MouseMoved));
	}

	void Update()
	{
		pos += MoveDir * Time::GetDeltaTime();
	}

private:
	void Move(const glm::vec2& movedir)
	{
		MoveDir += movedir;
	}

	void MouseMoved(glm::vec2& pos)
	{

	}
};

class ExampleLayer : public Engine::Layer
{
public:
	Engine::ShaderLibrary m_ShaderLib;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer;
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer;

	glm::vec3 m_Position;

	Player m_Player;

	bool set = false;
	ExampleLayer()
		: Layer("Example"), m_Position(0.0f)
	{

		m_Player.SetPlayerInput(m_InputManeger);


		m_VertexArray = Engine::VertexArray::Create();

		float vertices[4 * 3] =
		{
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f
		};

		m_VertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
		};

		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 2, 1, 0,  0, 3, 2 };

		m_IndexBuffer = Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		Engine::Ref<Engine::Shader::ShaderSorce> src = std::make_shared<Engine::Shader::ShaderSorce>();

		auto FlatShader = m_ShaderLib.Load("FlatShader", "Assets/Shaders/FlatColorShader.glsl");
	
		FlatShader->Bind();
	}

	void OnUpdate() override
	{
		m_Player.Update();

		auto FlatShader = m_ShaderLib.Get("FlatShader");

		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_Player);
		FlatShader->UploadUniformFloat4("u_Color", { 1.0f, 1.0f, 1.0f, 1.0f });
		Engine::Renderer::Submit(m_VertexArray, FlatShader, glm::translate(glm::mat4(1.0f), m_Position));

		//Engine::Renderer::Flush();
	}

	void OnEvent(Engine::Event& event) override
	{
		Super::OnEvent(event);
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