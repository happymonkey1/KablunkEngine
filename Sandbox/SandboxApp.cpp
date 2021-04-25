#include <kablunkpch.h>
#include <kablunk.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//#include "imgui.h"
class ExampleLayer : public Kablunk::Layer {
public:
	ExampleLayer()
		: Layer("Example"), m_Camera{ -1.6f, 1.6f, -0.9f, 0.9f }, m_CameraPosition{ 0.0f }, m_TrianglePosition{ 0.0f }, m_TileAColor{ 0.8f, 0.2f, 0.3f}, m_TileBColor{ 0.2f, 0.3f, 0.8f}
	{

		m_TriangleVA.reset(Kablunk::VertexArray::Create());
		m_SquareVA.reset(Kablunk::VertexArray::Create());

		float vertices[3 * 7]
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 0.5f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 0.5f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 0.5f
		};

		float sqrVertices[5 * 4]
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Kablunk::Ref<Kablunk::VertexBuffer> triangleVB;
		triangleVB.reset(Kablunk::VertexBuffer::Create(vertices, sizeof(vertices)));

		Kablunk::Ref<Kablunk::VertexBuffer> squareVB;
		squareVB.reset(Kablunk::VertexBuffer::Create(sqrVertices, sizeof(sqrVertices)));

		triangleVB->SetLayout({
			{ Kablunk::ShaderDataType::Float3, "a_Position" },
			{ Kablunk::ShaderDataType::Float4, "a_Color" }
			});
		m_TriangleVA->AddVertexBuffer(triangleVB);

		squareVB->SetLayout({
			{ Kablunk::ShaderDataType::Float3, "a_Position" },
			{ Kablunk::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t indices[3]{ 0, 1, 2 };
		Kablunk::Ref<Kablunk::IndexBuffer> triangleIB;
		triangleIB.reset(Kablunk::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_TriangleVA->SetIndexBuffer(triangleIB);

		uint32_t squareIndices[6]{ 0, 1, 2, 2, 3, 0 };
		Kablunk::Ref<Kablunk::IndexBuffer> squareIB;
		squareIB.reset(Kablunk::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 450 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}

		)";

		std::string fragmentSrc = R"(
			#version 450 core
			
			layout(location = 0) out vec4 o_Color;
			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				o_Color = vec4(v_Position * 0.5 + 0.5, 0.5);
				o_Color = v_Color;
			}

		)";

		// ==================
		//   Texture Shader
		// ==================

		std::string textureVertexSrc = R"(
			#version 450 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			out vec2 v_TexCoord;

			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}

		)";

		std::string textureFragmentSrc = R"(
			#version 450 core
			
			layout(location = 0) out vec4 o_Color;
			
			in vec2 v_TexCoord;
			

			uniform sampler2D u_Texture;

			void main()
			{
				o_Color = texture(u_Texture, v_TexCoord);
			}

		)";

		// ===============

		std::string blueVertexSrc = R"(
			#version 450 core
			
			layout(location = 0) in vec3 a_Position;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform  * vec4(a_Position, 1.0);
				v_Position = a_Position;
			}

		)";

		std::string flatColorFragmentSrc = R"(
			#version 450 core
			
			layout(location = 0) out vec4 o_Color;
			in vec3 v_Position;
			uniform vec3 u_Color;

			void main()
			{
				o_Color = vec4(u_Color, 1.0f);
			}

		)";

		m_TriangleShader.reset(Kablunk::Shader::Create(vertexSrc, fragmentSrc));
		m_FlatColorShader.reset(Kablunk::Shader::Create(blueVertexSrc, flatColorFragmentSrc));
		m_TextureShader.reset(Kablunk::Shader::Create(textureVertexSrc, textureFragmentSrc));

		m_Texture = Kablunk::Texture2D::Create("assets/textures/missing_texture_64x.png");
		m_Logo = Kablunk::Texture2D::Create("assets/textures/test_logo2.png");

		std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	~ExampleLayer()
	{

	}


	void OnUpdate(Kablunk::Timestep ts) override {
		//KB_CLIENT_INFO("ExampleLayer::Update");

		

		// ==========
		//   Camera
		// ==========

		if (Kablunk::Input::IsKeyPressed(KB_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;


		if (Kablunk::Input::IsKeyPressed(KB_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;


		if (Kablunk::Input::IsKeyPressed(KB_KEY_Q))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_E))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kablunk::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Kablunk::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(.1f));

		std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_TriangleShader)->Bind();
		std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_TextureShader)->Bind();

		for (int y = 0; y < 20; ++y)
		{
			for (int x = 0; x < 20; ++x)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 squareTransform = glm::translate(glm::mat4(1.0f), pos) * scale;
				if (y % 2 == 0)
				{
					if (x % 2 == 0)
						std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_TileAColor);
					else
						std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_TileBColor);
				}
				else
				{
					if (x % 2 == 1)
						std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_TileAColor);
					else
						std::dynamic_pointer_cast<Kablunk::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_TileBColor);
				}
				Kablunk::Renderer::Submit(m_FlatColorShader, m_SquareVA, squareTransform);
			}
		}

		m_Texture->Bind();
		Kablunk::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_Logo->Bind();
		Kablunk::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.25f)));
		

		glm::mat4 triangleTransform = glm::translate(glm::mat4(1.0f), m_TrianglePosition);
		Kablunk::Renderer::Submit(m_TriangleShader, m_TriangleVA, triangleTransform);
		
		Kablunk::Renderer::EndScene();
	}

	void OnImGuiRender(Kablunk::Timestep ts) override {
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

		ImGui::Begin("Tileset Colors");

		ImGui::ColorEdit3("TileA", glm::value_ptr(m_TileAColor));
		ImGui::ColorEdit3("TileB", glm::value_ptr(m_TileBColor));

		ImGui::End();
	}

	void OnEvent(Kablunk::Event& e) {
		//KABLUNK_CLIENT_TRACE("{0}", e);

		/*Kablunk::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Kablunk::KeyPressedEvent>(KABLUNK_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));

		if (e.GetEventType() == Kablunk::EventType::KeyPressed) {
			Kablunk::KeyPressedEvent& keyEvent = (Kablunk::KeyPressedEvent&)e;

			if (keyEvent.GetKeyCode() == KB_KEY_TAB)
				KB_CLIENT_TRACE("Tab key pressed");
		}*/
	}
private:
	Kablunk::Ref<Kablunk::Shader> m_TriangleShader;
	Kablunk::Ref<Kablunk::Shader> m_FlatColorShader;
	Kablunk::Ref<Kablunk::Shader> m_TextureShader;

	Kablunk::Ref<Kablunk::VertexArray> m_TriangleVA;
	Kablunk::Ref<Kablunk::VertexArray> m_SquareVA;

	Kablunk::Ref<Kablunk::Texture2D> m_Texture;
	Kablunk::Ref<Kablunk::Texture2D> m_Logo;

	Kablunk::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	float m_CameraMoveSpeed = 5.0f;
	float m_CameraRotationSpeed = 10.0f;
	glm::vec3 m_TrianglePosition;

	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;

	glm::vec3 m_TileAColor;
	glm::vec3 m_TileBColor;
};


class Sandbox : public Kablunk::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());

		
	}

	~Sandbox() {

	}

private:
	
};

Kablunk::Application* Kablunk::CreateApplication() {
	return new Sandbox();
}