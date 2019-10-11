#include <Engine.h>
#include <iostream>
#include <fstream>
#include <functional>

class Player
{
public:
	Player(float width, float height)
	{
		cam = new Engine::OrthographicCamera(-width / 2, width / 2, -height / 2, height / 2); // crates a new camera with the given width and height of the camera
	}

	Engine::OrthographicCamera* cam = nullptr; // the camera
	Engine::Scope<InputControler> Input; // the input controler
	glm::vec2 MoveDir = { 0.0f, 0.0f }; // the direction to move in
	glm::vec2 pos = { 0.0f, 0.0f }; // the position of the player
	glm::vec2 rotation = { 0.0f, 0.0f }; // the rotation of the player
	float FOV = 3.14f / 4; // the fov
	float RunSpeed = 2.0f; // the run speed
	float WalkSpeed = 1.0f; // the walk speed
	float Speed = WalkSpeed; // the speed

	void SetPlayerInput(Engine::InputControlerManeger* maneger)
	{
		Input.reset(new InputControler(maneger)); // sets up the input controler

		Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 0.0f,  1.0f }))); // binds w to start moving forword
		Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 0.0f, -1.0f }))); // binds s to start moving back
		Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ -1.0f,  0.0f }))); // binds a to start moving left
		Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&Player::Move, glm::vec2({ 1.0f,  0.0f }))); // binds d to start moving right

		Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 0.0f,  1.0f }))); // binds w to end moving forword
		Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 0.0f, -1.0f }))); // binds s to end moving back
		Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ -1.0f,  0.0f }))); // binds a to end moving left
		Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&Player::Move, -glm::vec2({ 1.0f,  0.0f }))); // binds d to end moving right

		Input->BindEvent(KEYCODE_Q, KEY_PRESSED, BIND_AXIS(&Player::SetRun, true)); // starts runing
		Input->BindEvent(KEYCODE_Q, KEY_RELEASED, BIND_AXIS(&Player::SetRun, false)); // ends runing

		Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&Player::MouseMoved)); // binds the mouse delta 
	}

	void Update()
	{
		glm::vec2 moveFB(glm::cos(rotation.x), glm::sin(rotation.x)); // gets the world dir to move forword
		glm::vec2 moveRL(glm::sin(rotation.x), -glm::cos(rotation.x)); // gets the workd dir to move left 
		pos += ((MoveDir.y * moveFB) + (MoveDir.x * moveRL)) * Speed * Time::GetDeltaTime(); // moves the player in the given dir
	}

private:
	void Move(const glm::vec2& movedir)
	{
		MoveDir += movedir; // sets the direction to move
	}

	void MouseMoved(glm::vec2& pos)
	{
		rotation.x -= pos.x / 500; // sets the x rotation of the player
		rotation.y += pos.y; // sets the y rotation of the player
		rotation.y = glm::clamp(rotation.y, -1500.0f, 1500.0f); // clamps the y rotaion
	}

	void SetRun(bool v)
	{
		Speed = v ? RunSpeed : WalkSpeed; // sets the speed based on wether or not the player is runing
	}
};

class Ray
{
public:
	Ray(glm::vec2 pos = { 0.0f, 0.0f }, float r = 0.0f)
		: m_Position(pos), m_Rotation(r) // sets the position and rotation of the ray
	{}

	// finds if a wall is intersecting the ray
	glm::vec2 Intersecting(glm::vec2 p1, glm::vec2 p2)
	{
		glm::vec2 dir = { glm::cos(m_Rotation), glm::sin(m_Rotation) }; // gets the normelized dir of the ray

		// i dont understand the math that well just read the artical -> https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection <-
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

	glm::vec3 pos = { 0.0f, 0.0f, 0.0f }; // positoin fo the sprite
	float Width = 1.0f; // width fo the sprit

	int texture = -1; // testure to use

	int GetBilbordPoints(glm::vec3* p1, glm::vec3* p2, const Player& player)
	{	
		glm::vec2 vecToPlayer = glm::vec3(player.pos, 0.0f) - pos; // gest a vector from the sprite to the player
		float rotation = std::atan2(vecToPlayer.y, vecToPlayer.x); // gets the roation of the sprite to look at the player

		// creates the poits of of the sprites
		*p1 = glm::vec3(glm::sin(rotation), -glm::cos(rotation), 0.0f) * (Width / 2); 
		*p2 = -*p1; 

		return texture;
	}
};

void SaveMapData(std::vector<float> MapVertexArray, std::vector<uint32_t> MapIndexBuffer, const std::string& filePath)
{
	// opens a file to write to
	std::ofstream out;
	out.open(filePath, std::ios::out | std::ios::binary);
	if (out.is_open()) // checks if the file is open
	{
		int size[] = { MapVertexArray.size(),  MapIndexBuffer.size() };
		out.write((char*)(&size), sizeof(size)); // saves the size of the vertex and index arrays
		out.write(reinterpret_cast<const char*>(MapVertexArray.data()), size[0] * sizeof(float)); // saves the vertex data
		out.write(reinterpret_cast<const char*>(MapIndexBuffer.data()), size[1] * sizeof(uint32_t)); // saves the index data
	}
	out.close(); // closes the file
}

void LoadMapData(std::vector<float>* MapVertexArray, std::vector<uint32_t>* MapIndexBuffer, const std::string& filePath)
{
	// opens a file to read from
	std::ifstream in;
	in.open(filePath, std::ios::in | std::ios::binary);
	if (in.is_open()) // checks if the file is open
	{
		int size[] = { 0,0 };
		in.read((char*)(&size), sizeof(size)); // loads the size of the vertex and index array
		MapVertexArray->resize(size[0]); // sets the size of the vertex array
		MapIndexBuffer->resize(size[1]); // stes the size of the index array
		in.read((char*)(MapVertexArray->data()), size[0] * sizeof(float)); // loads the vertex data
		in.read((char*)(MapIndexBuffer->data()), size[1] * sizeof(uint32_t)); // loads the index data
	}
	in.close(); // closes the file
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
	Engine::ShaderLibrary m_ShaderLib; // the shader library
	Engine::Ref<Engine::VertexArray> m_VertexArray; // the vertex array for the column
	Engine::Ref<Engine::VertexBuffer> m_VertexBuffer; // the vertex buffer for the column
	Engine::Ref<Engine::IndexBuffer> m_IndexBuffer; // the index buffer for the column
	Engine::Ref<Engine::Texture2D> m_Texture; // the texture atls for the map

	std::vector<float> m_MapVertexArray; // the map vetex array
	std::vector<uint32_t> m_MapIndexBuffer; // the map index array

	std::vector<Sprite> m_Sprites; // the sprites in the map

	Player* m_Player; // the player

	const float Reselution = 400; // the resolution
	int height = 0; // the height of the screen
	int width = 0; // the widht of the screen

	ExampleLayer()
		: Layer("Example")
	{
		height = Engine::Application::Get().GetWindow().GetHeight(); // gets the height of the screen
		width = Engine::Application::Get().GetWindow().GetWidth(); // gets the width of the screen

		m_Player = new Player(width, height); // creates a player

		m_Player->SetPlayerInput(m_InputManeger); // sets the players input events


		m_VertexArray = Engine::VertexArray::Create(); // creates a vertex array for the column

		// psoition and UV coords for the column
		float vertices[4 * 5] =
		{
			0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			0.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};

		m_VertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices)); // creates the vectex buffer

		// creates the layout of the verex data
		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"}
		};

		m_VertexBuffer->SetLayout(layout); // sets the layout for the vertex data

		m_VertexArray->AddVertexBuffer(m_VertexBuffer); // sets the vertex array to use the vertex buffer

		uint32_t indeces[] = { 2, 1, 0,  0, 3, 2 }; // index array

		m_IndexBuffer = Engine::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)); // creats the index buffer
		m_VertexArray->SetIndexBuffer(m_IndexBuffer); // sets the index buffer for the vertex array

		auto FlatShader = m_ShaderLib.Load("FlatShader", "Assets/Shaders/FlatColorShader.shader"); // creates a shader and adds it to the shader library

		FlatShader->Bind(); // binds the the shader

		// map vertex array
		m_MapVertexArray =
		{
			-5.0f, -5.0f, 0.15f,
			-5.0f,  5.0f, 0.0f,
			 5.0f,  5.0f, 0.0f,
			 5.0f, -5.0f, 0.0f,
		};

		m_MapIndexBuffer = { 0,1,  1,2,  2,3,  3,0 }; // map index buffer

		m_Sprites.push_back(Sprite({ 0.0f, 0.0f, 0.0f }, 1.0f, 0)); // sprites 

		SaveMapData(m_MapVertexArray, m_MapIndexBuffer, "Assets/Map/Map.txt"); // saves the map data to a file

		LoadMapData(&m_MapVertexArray, &m_MapIndexBuffer, "Assets/Map/Map.txt"); // loads the map data from a file
	}

	std::vector<intersectionData> FindClosest(Ray ray, std::vector<uint32_t> i)
	{
		std::vector<intersectionData> intersections; // all the intersections with the ray
		for (int j = 0; j < i.size() / 2; j++) // itorates over all walls in the map
		{
			uint32_t p1 = i[j * 2]; // gets the index for the first point in the wall
			uint32_t p2 = i[(j * 2) + 1]; // gets the index for the second poiont in the wall
			glm::vec3 point1(m_MapVertexArray[p1 * 3], m_MapVertexArray[(p1 * 3) + 1], m_MapVertexArray[(p1 * 3) + 2]); // gets the first point
			glm::vec3 point2(m_MapVertexArray[p2 * 3], m_MapVertexArray[(p2 * 3) + 1], m_MapVertexArray[(p2 * 3) + 2]); // gets the second point

			glm::vec2 pos = ray.Intersecting(point1, point2); // gets the intersection point
			const float dist = glm::distance(m_Player->pos, pos); // gets the distance from the player to the intersection point
			if (dist == 0 || pos == glm::vec2(0.0f, 0.0f)) // checks if the intersection point is valid
				continue;
			else
			{
				glm::vec3 top = point1.z > point2.z ? point1 : point2; // gets the point on the wall that is the tallest
				glm::vec3 bottom = point1.z > point2.z ? point2 : point1; // gets the point on the wall that is lowest
				float v0 = bottom.z; // gets the height of the bottom point on the wall
				float v1 = top.z; // gets the height of the top point on the wall
				float t = glm::distance(glm::vec2(bottom.x, bottom.y), pos); // gets the interpleation amount
				float interpulated = v0 + t * (v1 - v0); // linearly interpelates between the poitns height 
				glm::vec3 point = glm::vec3(pos.x, pos.y, interpulated); // creates the point to draw the colome at
				intersectionData data = { 0 }; // initalizes that intersection data
				data.dist = dist; // the distance form the player
				data.point = point; // the point of intersection
				data.texture = -1; // the texture on the texture atles to use fo the column
				intersections.push_back(data); // pushes the intersection data to the array
			}
		}
		for (int j = 0; j < m_Sprites.size(); j++) // itorates over all sprites in the map
		{
			Sprite& sprite = m_Sprites[j]; // gets the sprite
			glm::vec3* point1 = new glm::vec3(); // the left point of the sprite
			glm::vec3* point2 = new glm::vec3(); // the right point of the sprite
			int texture = sprite.GetBilbordPoints(point1, point2, *m_Player); // gets the billborded point of the sprite

			glm::vec2 pos = ray.Intersecting(*point1, *point2); // finds the intersection point on the sprite
			const float dist = glm::distance(m_Player->pos, pos); // gets the distance to the sprite
			if (dist == 0 || pos == glm::vec2(0.0f, 0.0f)) // checks if the the point is valid
				continue;
			else
			{
				glm::vec3 point = glm::vec3(pos.x, pos.y, sprite.pos.z);
				intersectionData data = { 0 }; // initalizes that intersection data
				data.dist = dist; // the distance form the player
				data.point = point; // the point of intersection
				data.texture = texture; // the texture on the texture atles to use fo the column
				intersections.push_back(data);// pushes the intersection data to the array
			}
		}
		// sortes the intersection data based on the distance
		std::sort(intersections.begin(), intersections.end(), [](intersectionData a, intersectionData b)
			{
				return a.dist > b.dist;
			});
		return intersections;
	}

	void OnUpdate() override
	{
		m_Player->Update();

		auto FlatShader = m_ShaderLib.Get("FlatShader"); // gets the shader the columns are going to use

		Engine::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 }); // sets the clear color
		Engine::RenderCommand::Clear(); // cleares the screen

		Engine::Renderer::BeginScene(*m_Player->cam); // sets the camera for the renderer to use

		// ---------------- Map Rendering ------------------ //
		float deltax = width / Reselution; // change in column poition on the screen
		glm::vec3 position(-width / 2, 0.0f, 0.0f); // position to draw the column
		for (int x = 0; x < Reselution; x++) // itorates over all column on the screen
		{
			float rotation = (m_Player->rotation.x + (m_Player->FOV / 2) - ((float)x * (m_Player->FOV / Reselution))); // gets the rotation of the ray
			Ray ray(m_Player->pos, rotation); // creates the ray

			std::vector<intersectionData> intersections = FindClosest(ray, m_MapIndexBuffer); // finds all intersections with the map

			for (int i = 0; i < intersections.size(); i++) // itorates over all intersections
			{
				position.y = m_Player->rotation.y; // sets the y component of the draw position to the players up/down look to emulate looking up/down
				auto intersection = intersections[i]; // the current intersection
				float yScale = height / ((float)intersection.dist * cos(rotation - m_Player->rotation.x)); // the scale of the column
				position.y += intersection.point.z * yScale; // sets the hight of the wall

				glm::mat4 scale = glm::scale(glm::mat4(1.0f), { deltax, yScale, 0.0f }); // creates the scale matrex for the column

				float color = yScale / 350; // gets the gray scall color to draw the column at

				FlatShader->UploadUniformFloat4("u_Color", { color, color, color, 1.0f }); // sends the color to the shader
				Engine::Renderer::Submit(m_VertexArray, FlatShader, glm::translate(glm::mat4(1.0f), position) * scale); // uploads the geometry of the column that is being drawn
			}
			position.x += deltax; // sets the x poition of the draw position for the next column 
		}
		// ---------------- End Map Rendering ------------------ //

		Engine::Renderer::EndScene();

		//Engine::Renderer::Flush();
		//DEBUG_INFO("FPS: {0}", 1/Engine::Time::GetDeltaTime());
	}

	void OnEvent(Engine::Event& event) override
	{
		Super::OnEvent(event); // sends the event to the layer to be used by the layer's input maneger
	}
};

class Sandbox : public Engine::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer()); // creates a new layer
	}

	~Sandbox()
	{

	}

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox(); // creates a game
}