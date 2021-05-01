#include "Sandbox2D.h"

// TODO : ABSTRACT!
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController{ 1.7778f, true }
{

}

void Sandbox2D::OnAttach()
{
	m_SquareVA.reset(Kablunk::VertexArray::Create());

	float sqrVertices[5 * 4]
	{
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	Kablunk::Ref<Kablunk::VertexBuffer> squareVB;
	squareVB.reset(Kablunk::VertexBuffer::Create(sqrVertices, sizeof(sqrVertices)));

	squareVB->SetLayout({
		{ Kablunk::ShaderDataType::Float3, "a_Position" },
		{ Kablunk::ShaderDataType::Float2, "a_TexCoord" }
		});
	m_SquareVA->AddVertexBuffer(squareVB);

	uint32_t squareIndices[6]{ 0, 1, 2, 2, 3, 0 };
	Kablunk::Ref<Kablunk::IndexBuffer> squareIB;
	squareIB.reset(Kablunk::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

	m_SquareVA->SetIndexBuffer(squareIB);


	// ===============

	auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

	m_Texture = Kablunk::Texture2D::Create("assets/textures/missing_texture_64x.png");
	m_Logo = Kablunk::Texture2D::Create("assets/textures/kablunk_logo.png");

	std::dynamic_pointer_cast<Kablunk::OpenGLShader>(textureShader)->Bind();
	std::dynamic_pointer_cast<Kablunk::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Kablunk::Timestep ts)
{
	// ==========
	//   Update
	// ==========

	m_CameraController.OnUpdate(ts);

	// ==========
	//   Render
	// ==========

	Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Kablunk::RenderCommand::Clear();

	Kablunk::Renderer::BeginScene(m_CameraController.GetCamera());

	auto textureShader = m_ShaderLibrary.Get("Texture");
	std::dynamic_pointer_cast<Kablunk::OpenGLShader>(textureShader)->Bind();


	m_Texture->Bind();
	Kablunk::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
	m_Logo->Bind();
	Kablunk::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.25f)));


	/*glm::mat4 triangleTransform = glm::translate(glm::mat4(1.0f), m_TrianglePosition);
	Kablunk::Renderer::Submit(m_TriangleShader, m_TriangleVA, triangleTransform);*/

	Kablunk::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender(Kablunk::Timestep ts)
{
	if (m_ImguiUpdateCounter >= m_ImguiUpdateCounterMax)
	{
		float miliseconds = ts.GetMiliseconds();
		m_ImguiDeltaTime = miliseconds;
		m_ImguiFPS = 1000.0f / miliseconds;
		m_ImguiUpdateCounter -= m_ImguiUpdateCounterMax;
	}
	else
		m_ImguiUpdateCounter += ts.GetMiliseconds() / 1000.0f;

	ImGui::Begin("Debug Information");

	ImGui::Text("Frame time: %.*f", 4, m_ImguiDeltaTime);
	ImGui::Text("FPS: %.*f", 4, m_ImguiFPS);

	ImGui::End();
}

void Sandbox2D::OnEvent(Kablunk::Event& e)
{
	m_CameraController.OnEvent(e);
}
