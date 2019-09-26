#include <Engine.h>
#include <iostream>
#include <fstream>


class Player
{
public:
	Player(float width, float height)
	{
		cam = new Engine::OrthographicCamera(-width / 2, width / 2, -height / 2, height / 2);
	}

	Engine::OrthographicCamera* cam = nullptr;
	Engine::Scope<InputControler> Input;
	float Speed = 1.0f;
	glm::vec2 MoveDir = { 0.0f, 0.0f };
	glm::vec2 pos = { 0.0f, 0.0f };
	float rotation = 0.0f;
	float y = 0.0f;
	float FOV = 3.14f / 4;
	float Speeed = 0.8f;

	void SetPlayerInput(Engine::InputControlerManeger* maneger)
	{
		Input.reset(new InputControler(maneger));

		Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ -1.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 1.0f,  0.0f })));

		Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 0.0f,  1.0f })));
		Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 0.0f, -1.0f })));
		Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ -1.0f,  0.0f })));
		Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 1.0f,  0.0f })));

		Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&Player::MouseMoved));
	}

	void Update()
	{
		glm::vec2 moveFB(glm::cos(rotation), glm::sin(rotation));
		glm::vec2 moveRL(glm::sin(rotation), -glm::cos(rotation));
		pos += ((MoveDir.y * moveFB) + (MoveDir.x * moveRL)) * Speed * Time::GetDeltaTime();
	}

private:
	void Move(const glm::vec2& movedir)
	{
		MoveDir += movedir;
	}

	void MouseMoved(glm::vec2& pos)
	{
		rotation -= pos.x / 500;
		y += pos.y;
		y = glm::clamp(y, -1500.0f, 1500.0f);
	}
};

class Ray
{
public:
	Ray(glm::vec2 pos = { 0.0f, 0.0f }, float r = 0.0f)
		: m_Position(pos), m_Rotation(r)
	{}

	glm::vec2 Intersecting(glm::vec2 p1, glm::vec2 p2)
	{
		glm::vec2 dir = { glm::cos(m_Rotation), glm::sin(m_Rotation) };

		const float x1 = p1.x;
		const float y1 = p1.y;
		const float x2 = p2.x;
		const float y2 = p2.y;

		const float x3 = m_Position.x;
		const float y3 = m_Position.y;
		const float x4 = m_Position.x + dir.x;
		const float y4 = m_Position.y + dir.y;

		const float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		if (den == 0.0f)
		{
			return {};
		}
		float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;

		float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;
		if (t > 0 && t < 1 && u > 0)
		{
			float x = x1 + t * (x2 - x1);
			float y = y1 + t * (y2 - y1);
			return { x, y };
		}
		else
		{
			return {};
		}
	}

	glm::vec2 m_Position;
	float m_Rotation;
};

class ExampleLayer : public Engine::Layer
{
public:
	Engine::ShaderLibrary m_ShaderLib;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer;
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer;

	std::vector<float> m_MapVertexArray;
	std::vector<uint32_t> m_MapIndexBuffer;

	Player* m_Player;

	float Reselution = 200;
	int height = 0;
	int width = 0;

	bool set = false;
	ExampleLayer()
		: Layer("Example")
	{
		height = Engine::Application::Get().GetWindow().GetHeight();
		width = Engine::Application::Get().GetWindow().GetWidth();

		m_Player = new Player(width, height);

		m_Player->SetPlayerInput(m_InputManeger);


		m_VertexArray = Engine::VertexArray::Create();

		float vertices[4 * 3] =
		{
			0.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
			0.0f,  1.0f, 0.0f
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

		m_MapVertexArray =
		{
			-5.0f, -5.0f,
			-5.0f,  5.0f,
			 5.0f,  5.0f,
			 5.0f, -5.0f,
			 0.0f, -5.0f,
			-5.0f,  0.0f
		};

		m_MapIndexBuffer = { 0,1,  1,2,  2,3,  3,0, 4,5 };

		Engine::Ref<Engine::Shader::ShaderSorce> src = std::make_shared<Engine::Shader::ShaderSorce>();

		auto FlatShader = m_ShaderLib.Load("FlatShader", "Assets/Shaders/FlatColorShader.glsl");

		FlatShader->Bind();

		std::ofstream myfile;
		myfile.open("Assets/Map/Map.m", std::ios::out | std::ios::binary);
		if (myfile.is_open())
		{
			const char mapSize[2] = { m_MapVertexArray.size(), m_MapIndexBuffer.size() };
			myfile.write(mapSize, 2);
			myfile << *(char*)m_MapVertexArray.data();
			myfile << *(char*)m_MapIndexBuffer.data();
		}
		myfile.close();
	}

	float FindClosest(Ray ray, std::vector<uint32_t> i)
	{
		float closest = MAXINT;
		for (int j = 0; j < i.size() / 2; j++)
		{
			float p1 = i[j * 2];
			float p2 = i[(j * 2) + 1];
			glm::vec2 point1(m_MapVertexArray[p1 * 2], m_MapVertexArray[(p1 * 2) + 1]);
			glm::vec2 point2(m_MapVertexArray[p2 * 2], m_MapVertexArray[(p2 * 2) + 1]);

			glm::vec2 pos = ray.Intersecting(point1, point2);
			const float dist = glm::distance(m_Player->pos, pos);
			if (dist == 0 || pos == glm::vec2(0.0f, 0.0f))
				continue;
			if (dist < closest)
			{
				closest = dist;
			}
		}
		return closest;
	}

	void OnUpdate() override
	{
		m_Player->Update();

		auto FlatShader = m_ShaderLib.Get("FlatShader");

		Engine::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(*m_Player->cam);

		float deltax = width / Reselution;
		glm::vec3 position(-width / 2, 0.0f, 0.0f);
		for (int x = 0; x < Reselution; x++)
		{
			position.y = m_Player->y;
			float rotation = (m_Player->rotation + (m_Player->FOV / 2) - ((float)x * (m_Player->FOV / Reselution)));
			Ray ray(m_Player->pos, rotation);

			float closest = FindClosest(ray, m_MapIndexBuffer) * cos(rotation-m_Player->rotation);

			float nscale = height / ((float)closest);

			//DEBUG_INFO("{0}, {1}      PlayerPos: {2}, {3}       PlayerRotaion: {4}", closest, nscale, m_Player->pos.x, m_Player->pos.y, m_Player->rotation);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), { deltax, nscale, 0.0f });

			float color = nscale / 300;
			FlatShader->UploadUniformFloat4("u_Color", { color, color, color, 1.0f });
			Engine::Renderer::Submit(m_VertexArray, FlatShader, glm::translate(glm::mat4(1.0f), position) * scale);
			position.x += deltax;
		}

		Engine::Renderer::EndScene();

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