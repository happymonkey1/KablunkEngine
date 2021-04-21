#include <kablunkpch.h>
#include <kablunk.h>


//#include "imgui.h"
class ExampleLayer : public Kablunk::Layer {
public:
	ExampleLayer() 
		: Layer("Example"), m_Camera{-1.6f, 1.6f, -0.9f, 0.9f}, m_CameraPosition{0.0f}
	{
		m_TriangleVA.reset(Kablunk::VertexArray::Create());
		m_SquareVA.reset(Kablunk::VertexArray::Create());

		float vertices[3 * 7]
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		float sqrVertices[3 * 4]
		{
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};

		std::shared_ptr<Kablunk::VertexBuffer> triangleVB;
		triangleVB.reset(Kablunk::VertexBuffer::Create(vertices, sizeof(vertices)));

		std::shared_ptr<Kablunk::VertexBuffer> squareVB;
		squareVB.reset(Kablunk::VertexBuffer::Create(sqrVertices, sizeof(sqrVertices)));

		triangleVB->SetLayout({
			{ Kablunk::ShaderDataType::Float3, "a_Position" },
			{ Kablunk::ShaderDataType::Float4, "a_Color" }
			});
		m_TriangleVA->AddVertexBuffer(triangleVB);

		squareVB->SetLayout({
			{ Kablunk::ShaderDataType::Float3, "a_Position" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t indices[3]{ 0, 1, 2 };
		std::shared_ptr<Kablunk::IndexBuffer> triangleIB;
		triangleIB.reset(Kablunk::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_TriangleVA->SetIndexBuffer(triangleIB);

		uint32_t squareIndices[6]{ 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Kablunk::IndexBuffer> squareIB;
		squareIB.reset(Kablunk::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 450 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}

		)";

		std::string fragmentSrc = R"(
			#version 450 core
			
			layout(location = 0) out vec4 o_Color;
			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				o_Color = vec4(v_Position * 0.5 + 0.5, 1.0);
				o_Color = v_Color;
			}

		)";

		std::string blueVertexSrc = R"(
			#version 450 core
			
			layout(location = 0) in vec3 a_Position;
			
			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
				v_Position = a_Position;
			}

		)";

		std::string blueFragmentSrc = R"(
			#version 450 core
			
			layout(location = 0) out vec4 o_Color;
			in vec3 v_Position;

			void main()
			{
				o_Color = vec4(0.2, 0.3, 0.8, 1.0);
			}

		)";

		m_TriangleShader.reset(Kablunk::Shader::Create(vertexSrc, fragmentSrc));
		m_BlueShader.reset(Kablunk::Shader::Create(blueVertexSrc, blueFragmentSrc));
	}

	void OnUpdate(Kablunk::Timestep ts) override {
		//KB_CLIENT_INFO("ExampleLayer::Update");

		KB_CLIENT_TRACE("Delta time: {0}s ({1}ms)", ts.GetSeconds(), ts.GetMiliseconds());

		if (Kablunk::Input::IsKeyPressed(KB_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;


		if (Kablunk::Input::IsKeyPressed(KB_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (Kablunk::Input::IsKeyPressed(KB_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		else if (Kablunk::Input::IsKeyPressed(KB_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Kablunk::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Kablunk::Renderer::BeginScene(m_Camera);

		Kablunk::Renderer::Submit(m_BlueShader, m_SquareVA);
		Kablunk::Renderer::Submit(m_TriangleShader, m_TriangleVA);

		Kablunk::Renderer::EndScene();

		if (Kablunk::Input::IsKeyPressed(KB_KEY_TAB))
			KB_CLIENT_INFO("Tab key is pressed");
	}

	virtual void OnImGuiRender() override {
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
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
	std::shared_ptr<Kablunk::Shader> m_TriangleShader;
	std::shared_ptr<Kablunk::Shader> m_BlueShader;

	std::shared_ptr<Kablunk::VertexArray> m_TriangleVA;
	std::shared_ptr<Kablunk::VertexArray> m_SquareVA;

	Kablunk::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	float m_CameraMoveSpeed = 1.0f;
	float m_CameraRotationSpeed = 10.0f;
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