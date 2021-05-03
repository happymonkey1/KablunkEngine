#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;

		// TODO color, texid
	};


	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;

		Ref <VertexArray> QuadVertexArray;
		Ref <VertexBuffer> QuadVertexBuffer;
		Ref <Shader> TextureShader;
		Ref <Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
	};

	

	static Renderer2DData s_RendererData;

	void Renderer2D::Init()
	{
		KB_PROFILE_FUNCTION();

		s_RendererData.QuadVertexArray = VertexArray::Create();

		s_RendererData.QuadVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVertices * sizeof(QuadVertex));

		s_RendererData.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"},
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_RendererData.QuadVertexArray->AddVertexBuffer(s_RendererData.QuadVertexBuffer);

		s_RendererData.QuadVertexBufferBase = new QuadVertex[s_RendererData.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_RendererData.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_RendererData.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + static_cast<uint32_t>(0);
			quadIndices[i + 1] = offset + static_cast<uint32_t>(1);
			quadIndices[i + 2] = offset + static_cast<uint32_t>(2);
										  						 
			quadIndices[i + 3] = offset + static_cast<uint32_t>(2);
			quadIndices[i + 4] = offset + static_cast<uint32_t>(3);
			quadIndices[i + 5] = offset + static_cast<uint32_t>(0);

			offset += static_cast<uint32_t>(4);
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_RendererData.MaxIndices);
		s_RendererData.QuadVertexArray->SetIndexBuffer(quadIB);
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

		delete[] s_RendererData.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		KB_PROFILE_FUNCTION();



		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_RendererData.QuadIndexCount = 0;
		s_RendererData.QuadVertexBufferPtr = s_RendererData.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene()
	{
		KB_PROFILE_FUNCTION();

		

		Flush();
	}

	void Renderer2D::Flush()
	{
		KB_PROFILE_FUNCTION();

		if (s_RendererData.QuadIndexCount == 0)
			return;

		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.QuadVertexBufferPtr - (uint8_t*)s_RendererData.QuadVertexBufferBase);
		s_RendererData.QuadVertexBuffer->SetData(s_RendererData.QuadVertexBufferBase, dataSize);

		RenderCommand::DrawIndexed(s_RendererData.QuadVertexArray, s_RendererData.QuadIndexCount);
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

		/*constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			
		}*/

		s_RendererData.QuadVertexBufferPtr->Position = position;
		s_RendererData.QuadVertexBufferPtr->Color = color;
		s_RendererData.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_RendererData.QuadVertexBufferPtr++;

		s_RendererData.QuadVertexBufferPtr->Position = {position.x + size.x, position.y, 0.0f};
		s_RendererData.QuadVertexBufferPtr->Color = color;
		s_RendererData.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_RendererData.QuadVertexBufferPtr++;

		s_RendererData.QuadVertexBufferPtr->Position = {position.x + size.x, position.y + size.y, 0.0f};
		s_RendererData.QuadVertexBufferPtr->Color = color;
		s_RendererData.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_RendererData.QuadVertexBufferPtr++;

		s_RendererData.QuadVertexBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		s_RendererData.QuadVertexBufferPtr->Color = color;
		s_RendererData.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_RendererData.QuadVertexBufferPtr++;

		s_RendererData.QuadIndexCount += 6;
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