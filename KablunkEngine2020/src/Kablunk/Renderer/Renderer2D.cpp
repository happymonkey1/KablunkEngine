#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{
	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		Ref <VertexArray> QuadVertexArray;
		Ref <Shader> TextureShader;
		Ref <Texture2D> WhiteTexture;
	};

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;

		// TODO color, texid
	};

	static Renderer2DData s_RendererData;

	void Renderer2D::Init()
	{
		KB_PROFILE_FUNCTION();

		s_RendererData.QuadVertexArray = VertexArray::Create();

		Ref<VertexBuffer> quadVB = VertexBuffer::Create(s_RendererData.MaxVertices * sizeof(QuadVertex));

		quadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});
		s_RendererData.QuadVertexArray->AddVertexBuffer(quadVB);

		uint32_t* quadIndices = new uint32_t[s_RendererData.MaxIndices];

		Ref<IndexBuffer> squareIB = IndexBuffer::Create(quadIndices, s_RendererData.MaxIndices);
		s_RendererData.QuadVertexArray->SetIndexBuffer(squareIB);
		delete[] quadIndices;

		s_RendererData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RendererData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_RendererData.TextureShader = Shader::Create("assets/shaders/Texture.glsl");

		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		KB_PROFILE_FUNCTION();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		KB_PROFILE_FUNCTION();

		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
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
		KB_PROFILE_FUNCTION();

		s_RendererData.TextureShader->SetFloat4("u_Color", color);
		s_RendererData.TextureShader->SetFloat("u_TilingFactor", 1.0f);
		s_RendererData.WhiteTexture->Bind();

		glm::mat4 transform;
		{
			KB_PROFILE_SCOPE("Matrix math - Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)");
			transform = glm::translate(glm::mat4{ 1.0f }, position)
				* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });
		}
		s_RendererData.TextureShader->SetMat4("u_Transform", transform);

		s_RendererData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData.QuadVertexArray);
	}

	// ==========================
	//   Draw Quad with Texture
	// ==========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		KB_PROFILE_FUNCTION();

		s_RendererData.TextureShader->SetFloat4("u_Color", tintColor);
		s_RendererData.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		texture->Bind();


		glm::mat4 transform;
		{
			KB_PROFILE_SCOPE("Matrix Math - Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)");
			transform = glm::translate(glm::mat4{ 1.0f }, position)
				* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });
		}
		s_RendererData.TextureShader->SetMat4("u_Transform", transform);

		

		s_RendererData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData.QuadVertexArray);
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
		KB_PROFILE_FUNCTION();

		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetFloat4("u_Color", color);
		s_RendererData.TextureShader->SetFloat("u_TilingFactor", 1.0f);
		s_RendererData.WhiteTexture->Bind();

		glm::mat4 transform;
		{
			KB_PROFILE_SCOPE("Matrix Math - Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation)");
			transform = glm::translate(glm::mat4{ 1.0f }, position)
				* glm::rotate(glm::mat4{ 1.0f }, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
				* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });
		}
		s_RendererData.TextureShader->SetMat4("u_Transform", transform);

		s_RendererData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData.QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 1.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		KB_PROFILE_FUNCTION();

		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetFloat4("u_Color", tintColor);
		s_RendererData.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		texture->Bind();

		glm::mat4 transform;
		{
			KB_PROFILE_SCOPE("Matrix Math - Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor)");
			transform = glm::translate(glm::mat4{ 1.0f }, position)
				* glm::rotate(glm::mat4{ 1.0f }, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
				* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });
		}
		s_RendererData.TextureShader->SetMat4("u_Transform", transform);

		s_RendererData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RendererData.QuadVertexArray);
	}

}