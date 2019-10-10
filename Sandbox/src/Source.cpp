#include <Engine.h>
#include <iostream>
#include <fstream>
#include <functional>

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
	glm::vec2 rotation = { 0.0f, 0.0f };
	float FOV = 3.14f / 4;
	float RunSpeed = 2.0f;
	float WalkSpeed = 1.0f;
	float Speeed = WalkSpeed;

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

		Input->BindEvent(KEYCODE_Q, KEY_PRESSED, BIND_AXIS(&Player::SetRun, true));
		Input->BindEvent(KEYCODE_Q, KEY_RELEASED, BIND_AXIS(&Player::SetRun, false));

		Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&Player::MouseMoved));
	}

	void Update()
	{
		glm::vec2 moveFB(glm::cos(rotation.x), glm::sin(rotation.x));
		glm::vec2 moveRL(glm::sin(rotation.x), -glm::cos(rotation.x));
		pos += ((MoveDir.y * moveFB) + (MoveDir.x * moveRL)) * Speed * Time::GetDeltaTime();
	}

private:
	void Move(const glm::vec2& movedir)
	{
		MoveDir += movedir;
	}

	void MouseMoved(glm::vec2& pos)
	{
		rotation.x -= pos.x / 500;
		rotation.y += pos.y;
		rotation.y = glm::clamp(rotation.y, -1500.0f, 1500.0f);
	}

	void SetRun(bool v)
	{
		Speed = v ? RunSpeed : WalkSpeed;
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

class Sprite
{
public:
	Sprite(glm::vec3 p = { 0.0f, 0.0f, 0.0f } , float w = 1.0f, int t = -1)
		: pos(p), Width(w), texture(t)
	{}

	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	float Width = 1.0f;

	int texture = -1;

	int GetBilbordPoints(glm::vec3* p1, glm::vec3* p2, const Player& player)
	{	
		glm::vec2 vecToPlayer = glm::vec3(player.pos, 0.0f) - pos;
		float rotation = std::atan2(vecToPlayer.y, vecToPlayer.x);

		*p1 = glm::vec3(glm::sin(rotation), -glm::cos(rotation), 0.0f) * (Width / 2);
		*p2 = -*p1;

		return texture;
	}
};

void SaveMapData(std::vector<float> MapVertexArray, std::vector<uint32_t> MapIndexBuffer)
{
	std::ofstream out;
	out.open("Assets/Map/Map.txt", std::ios::out | std::ios::binary);
	if (out.is_open())
	{
		int size[] = { MapVertexArray.size(),  MapIndexBuffer.size() };
		out.write((char*)(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(MapVertexArray.data()), size[0] * sizeof(float));
		out.write(reinterpret_cast<const char*>(MapIndexBuffer.data()), size[1] * sizeof(uint32_t));
	}
	out.close();
}

void LoadMapData(std::vector<float>* MapVertexArray, std::vector<uint32_t>* MapIndexBuffer)
{
	std::ifstream in;
	in.open("Assets/Map/Map.txt", std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		int size[] = { 0,0 };
		in.read((char*)(&size), sizeof(size));
		MapVertexArray->resize(size[0]);
		MapIndexBuffer->resize(size[1]);
		in.read((char*)(MapVertexArray->data()), size[0] * sizeof(float));
		in.read((char*)(MapIndexBuffer->data()), size[1] * sizeof(uint32_t));
	}
}

struct intersectionData
{
	float dist;
	glm::vec3 point;
	int texture;
};

class ExampleLayer : public Engine::Layer
{
public:
	Engine::ShaderLibrary m_ShaderLib;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer;
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer;
	Engine::Ref<Engine::Texture2D> m_Texture;

	std::vector<float> m_MapVertexArray;
	std::vector<uint32_t> m_MapIndexBuffer;

	std::vector<Sprite> m_Sprites;

	Player* m_Player;

	const float Reselution = 400;
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

		float vertices[4 * 5] =
		{
			0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			0.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};

		m_VertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));

		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"}
		};

		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 2, 1, 0,  0, 3, 2 };

		m_IndexBuffer = Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		m_MapVertexArray =
		{
			-5.0f, -5.0f, 0.15f,
			-5.0f,  5.0f, 0.0f,
			 5.0f,  5.0f, 0.0f,
			 5.0f, -5.0f, 0.0f,
		};

		m_MapIndexBuffer = { 0,1,  1,2,  2,3,  3,0 };

		m_Sprites.push_back(Sprite({ 0.0f, 0.0f, 0.0f }, 1.0f, 0));

		Engine::Ref<Engine::Shader::ShaderSorce> src = std::make_shared<Engine::Shader::ShaderSorce>();

		auto FlatShader = m_ShaderLib.Load("FlatShader", "Assets/Shaders/FlatColorShader.shader");

		FlatShader->Bind();

		SaveMapData(m_MapVertexArray, m_MapIndexBuffer);

		LoadMapData(&m_MapVertexArray, &m_MapIndexBuffer);
	}

	std::vector<intersectionData> FindClosest(Ray ray, std::vector<uint32_t> i)
	{
		std::vector<intersectionData> intersections;
		for (int j = 0; j < i.size() / 2; j++)
		{
			uint32_t p1 = i[j * 2];
			uint32_t p2 = i[(j * 2) + 1];
			glm::vec3 point1(m_MapVertexArray[p1 * 3], m_MapVertexArray[(p1 * 3) + 1], m_MapVertexArray[(p1 * 3) + 2]);
			glm::vec3 point2(m_MapVertexArray[p2 * 3], m_MapVertexArray[(p2 * 3) + 1], m_MapVertexArray[(p2 * 3) + 2]);

			glm::vec2 pos = ray.Intersecting(point1, point2);
			const float dist = glm::distance(m_Player->pos, pos);
			if (dist == 0 || pos == glm::vec2(0.0f, 0.0f))
				continue;
			else
			{
				glm::vec3 top = point1.z > point2.z ? point1 : point2;
				glm::vec3 bottom = point1.z > point2.z ? point2 : point1;
				float v0 = bottom.z;
				float v1 = top.z;
				float t = glm::distance(glm::vec2(bottom.x, bottom.y), pos);
				float interpulated = v0 + t * (v1 - v0);
				glm::vec3 point = glm::vec3(pos.x, pos.y, interpulated);
				intersectionData data = { 0 };
				data.dist = dist;
				data.point = point;
				data.texture = -1;
				intersections.push_back(data);
			}
		}
		for (int j = 0; j < m_Sprites.size(); j++)
		{
			Sprite& sprite = m_Sprites[j];
			glm::vec3* point1 = new glm::vec3();
			glm::vec3* point2 = new glm::vec3();
			int texture = sprite.GetBilbordPoints(point1, point2, *m_Player);

			glm::vec2 pos = ray.Intersecting(*point1, *point2);
			const float dist = glm::distance(m_Player->pos, pos);
			if (dist == 0 || pos == glm::vec2(0.0f, 0.0f))
				continue;
			else
			{
				glm::vec3 top = point1->z > point2->z ? *point1 : *point2;
				glm::vec3 bottom = point1->z > point2->z ? *point2 : *point1;
				float v0 = bottom.z;
				float v1 = top.z;
				float t = glm::distance(glm::vec2(bottom.x, bottom.y), pos);
				float interpulated = v0 + t * (v1 - v0);
				glm::vec3 point = glm::vec3(pos.x, pos.y, interpulated);
				intersectionData data = { 0 };
				data.dist = dist;
				data.point = point;
				data.texture = texture;
				intersections.push_back(data);
			}
		}
		std::sort(intersections.begin(), intersections.end(), [](intersectionData a, intersectionData b)
			{
				return a.dist > b.dist;
			});
		return intersections;
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
			float rotation = (m_Player->rotation.x + (m_Player->FOV / 2) - ((float)x * (m_Player->FOV / Reselution)));
			Ray ray(m_Player->pos, rotation);

			std::vector<intersectionData> intersections = FindClosest(ray, m_MapIndexBuffer);

			for (int i = 0; i < intersections.size(); i++)
			{
				position.y = m_Player->rotation.y;
				auto intersection = intersections[i];
				float yScale = height / ((float)intersection.dist * cos(rotation - m_Player->rotation.x));
				position.y += intersection.point.z * yScale;

				glm::mat4 scale = glm::scale(glm::mat4(1.0f), { deltax, yScale, 0.0f });

				float color = yScale / 350;

				FlatShader->UploadUniformFloat4("u_Color", { color, color, color, 1.0f });
				Engine::Renderer::Submit(m_VertexArray, FlatShader, glm::translate(glm::mat4(1.0f), position) * scale);
			}
			position.x += deltax;
		}

		Engine::Renderer::EndScene();

		//Engine::Renderer::Flush();
		//DEBUG_INFO("FPS: {0}", 1/Engine::Time::GetDeltaTime());
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