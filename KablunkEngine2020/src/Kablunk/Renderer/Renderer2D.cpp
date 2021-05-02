#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{
	struct Renderer2DStorage
	{
		Ref <VertexArray> QuadVertexArray;
		Ref <Shader> FlatColorShader;
		Ref <Shader> TextureShader;
	};

	static Renderer2DStorage* s_RendererData;

	void Renderer2D::Init()
	{
		s_RendererData = new Renderer2DStorage{};

		s_RendererData->QuadVertexArray = VertexArray::Create();

		float sqrVertices[5 * 4]
		{
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(sqrVertices, sizeof(sqrVertices));

		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});
		s_RendererData->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6]{ 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));

		s_RendererData->QuadVertexArray->SetIndexBuffer(squareIB);

		s_RendererData->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
		s_RendererData->TextureShader = Shader::Create("assets/shaders/Texture.glsl");

		s_RendererData->TextureShader->Bind();
		s_RendererData->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_RendererData;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_RendererData->FlatColorShader->Bind();
		s_RendererData->FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_RendererData->TextureShader->Bind();
		s_RendererData->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{

	}

	// ========================
	//   Draw Quad with Color
	// ========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		s_RendererData->FlatColorShader->Bind();
		s_RendererData->FlatColorShader->SetFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position) 
			* glm::scale(glm::mat4{1.0f}, glm::vec3{ size.x, size.y, 1.0f });
		s_RendererData->FlatColorShader->SetMat4("u_Transform", transform);

		s_RendererData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData->QuadVertexArray);
	}

	// ==========================
	//   Draw Quad with Texture
	// ==========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		s_RendererData->TextureShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
			* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });
		s_RendererData->TextureShader->SetMat4("u_Transform", transform);

		texture->Bind();

		s_RendererData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData->QuadVertexArray);
	}

	// ================================
	//   Draw Rotated Quad with Color
	// ================================

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, float rotation)
	{
		DrawRotatedQuad({ position.x, position.y, 1.0f }, size, color, rotation);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation)
	{
		s_RendererData->FlatColorShader->Bind();

		s_RendererData->FlatColorShader->SetFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position) 
			* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f }) 
			* glm::rotate(glm::mat4{ 1.0f }, glm::radians(rotation), {0.0f, 0.0f, 1.0f});

		s_RendererData->FlatColorShader->SetMat4("u_Transform", transform);

		s_RendererData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData->QuadVertexArray);
	}

}