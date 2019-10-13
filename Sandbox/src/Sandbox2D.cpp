#include "Sandbox2D.h"

Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
	m_Camera.reset(new Engine::OrthographicCameraControler(1.6f, 1.0f));

	m_Camera->SetPlayerInput(m_InputManeger);
}

void Sandbox2DLayer::OnAttach()
{
	m_VertexArray = Engine::VertexArray::Create();

	float vertices[4 * 5] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
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

	Engine::Ref<Engine::Shader::ShaderSorce> src = std::make_shared<Engine::Shader::ShaderSorce>();

	auto TextureShader = m_ShaderLib.Load("TextureShader", "Assets/Shaders/TextureShader.glsl");

	Engine::TextureAttribute* textAttrib = new Engine::TextureAttribute();
	textAttrib->AtlasRows = 2;

	m_Texture = Engine::Texture2D::Create("Assets/Images/UBIQ.png", *textAttrib);

	TextureShader->Bind();
	TextureShader->UploadUniformInt("u_Texture", 0);
	TextureShader->UploadUniformInt("u_AtlasRows", m_Texture->GetAttributes()->AtlasRows);
	TextureShader->UploadUniformFloat2("u_AtlasPos", m_Texture->AtlasIndexToPosition(2));
}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate()
{
	m_Camera->OnUpdate();

	auto TextureShader = m_ShaderLib.Get("TextureShader");

	Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Engine::RenderCommand::Clear();

	Engine::Renderer::BeginScene(*m_Camera->GetCamera());

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

	m_Texture->Bind(0);
	Engine::Renderer::Submit(m_VertexArray, TextureShader, glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f}));

	//Engine::Renderer::Flush();
}

void Sandbox2DLayer::OnImGuiRender()
{
	
}

void Sandbox2DLayer::OnEvent(Engine::Event& event)
{
	Super::OnEvent(event);

	m_Camera->OnEvent(event);
}
