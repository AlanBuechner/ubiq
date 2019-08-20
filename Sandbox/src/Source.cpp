#include <Engine.h>

class EditerCamera : public Engine::PerspectiveCamera
{
public:
	EditerCamera()
		: Super(60.0f, 16.0f/9.0f)
	{}

	Engine::Scope<InputControler> Input;
	float Speed = 1.0f;
	glm::vec3 MoveDir = { 0.0f, 0.0f, 0.0f };

	void SetPlayerInput(Engine::InputControlerManeger* maneger)
	{
		Input.reset(new InputControler(maneger));

		Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&EditerCamera::Move, glm::vec3({  0.0f, 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&EditerCamera::Move, glm::vec3({  0.0f, 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&EditerCamera::Move, glm::vec3({ -1.0f, 0.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&EditerCamera::Move, glm::vec3({  1.0f, 0.0f,  0.0f })));

		Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&EditerCamera::Move, -glm::vec3({  0.0f, 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&EditerCamera::Move, -glm::vec3({  0.0f, 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&EditerCamera::Move, -glm::vec3({ -1.0f, 0.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&EditerCamera::Move, -glm::vec3({  1.0f, 0.0f,  0.0f })));

		Input->BindMouseMoveEvent(MOUSE_DELTA_POSITON, BIND_MOUSEMOVE(&EditerCamera::MouseMoved));
	}

	void Update()
	{
		Translate(MoveDir * Speed * Time::GetDeltaTime());
		glm::vec3 pos = GetPosition();
		//DEBUG_INFO("{0}, {1}, {2}", pos.x, pos.y, pos.z);
	}

private:
	void Move(const glm::vec3& movedir)
	{
		glm::vec3 xMove = movedir.x * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 zMove = movedir.z * glm::vec3(0.0f, 0.0f, 1.0f);
		MoveDir += xMove + zMove;
	}

	void MouseMoved(glm::vec2& pos)
	{
		//DEBUG_INFO("{0}, {1}", pos.x, pos.y);
	}
};

class ExampleLayer : public Engine::Layer
{
public:
	InputControler* input;

	Engine::Ref<Engine::Shader> m_Shader, m_TextureShader;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer;
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer;
	Engine::Ref<Engine::Texture2D> m_Texture;

	glm::vec3 m_Position;

	EditerCamera m_Camera;

	bool set = false;
	ExampleLayer()
		: Layer("Example"), m_Position(0.0f)
	{
		input = new InputControler(m_InputManeger);

		m_Camera.SetPlayerInput(m_InputManeger);


		m_VertexArray = Engine::VertexArray::Create();

		float vertices[4 * 5] =
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		m_VertexBuffer.reset(Engine::VertexBuffer::Create(vertices, sizeof(vertices)));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"}
		};

		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 2, 1, 0,  0, 3, 2 };

		m_IndexBuffer.reset(Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		Engine::Shader::ShaderSorce src;

		/*src << Engine::Shader::LoadShader("C:\\Users\\alanj\\source\\repos\\GameEngine\\FlatColorShader.glsl");
		m_Shader.reset(Engine::Shader::Create(src.vertexShader, src.pixleShader));*/

		src << Engine::Shader::LoadShader("C:\\Users\\alanj\\source\\repos\\GameEngine\\TextureShader.glsl");
		m_TextureShader.reset(Engine::Shader::Create(src.vertexShader, src.pixleShader));

		m_Texture = Engine::Texture2D::Create("C:\\Users\\alanj\\source\\repos\\GameEngine\\logo\\UBIQ.png");
		m_Texture->Bind(0);

		m_TextureShader->Bind();
		m_TextureShader->UploadUniformInt("u_Texture", 0);

	}

	void OnUpdate() override
	{
		m_Camera.Update();

		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_Camera);

		/*glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

		glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 1.1f, y * 1.1f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
				if (x % 2 == 0)
					m_Shader->UploadUniformFloat4("u_Color", redColor);
				else
					m_Shader->UploadUniformFloat4("u_Color", blueColor);
				Engine::Renderer::Submit(m_VertexArray, m_Shader, transform);
			}
		}*/

		Engine::Renderer::Submit(m_VertexArray, m_TextureShader, glm::translate(glm::mat4(1.0f), m_Position));

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