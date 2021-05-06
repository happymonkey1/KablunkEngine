#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{

	Scope<Renderer2D::Renderer2DStats> Renderer2D::s_RendererStats = CreateScope<Renderer2D::Renderer2DStats>();

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		// TODO color, texid
	};


	struct Renderer2DData
	{
		const uint32_t MaxQuads = 100'000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;
		glm::vec4 QuadVertexPositions[4];

		Ref <VertexArray> QuadVertexArray;
		Ref <VertexBuffer> QuadVertexBuffer;
		Ref <Shader> TextureShader;
		Ref <Texture2D> WhiteTexture;

		uint32_t QuadCount = 0;
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		

		// TODO: change to asset handle when implemented
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; //0 = white texture
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
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float, "a_TexIndex" },
			{ ShaderDataType::Float, "a_TilingFactor" }
		});
		s_RendererData.QuadVertexArray->AddVertexBuffer(s_RendererData.QuadVertexBuffer);

		s_RendererData.QuadVertexBufferBase = new QuadVertex[s_RendererData.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_RendererData.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_RendererData.MaxIndices; i += 6)
		{
			quadIndices[i + uint32_t{ 0 }] = offset + uint32_t{0};
			quadIndices[i + uint32_t{ 1 }] = offset + uint32_t{1};
			quadIndices[i + uint32_t{ 2 }] = offset + uint32_t{2};
							  		 	  					  
			quadIndices[i + uint32_t{ 3 }] = offset + uint32_t{2};
			quadIndices[i + uint32_t{ 4 }] = offset + uint32_t{3};
			quadIndices[i + uint32_t{ 5 }] = offset + uint32_t{0};

			offset += uint32_t{ 4 };
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_RendererData.MaxIndices);
		s_RendererData.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_RendererData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RendererData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_RendererData.MaxTextureSlots];
		for (int32_t i = 0; i < s_RendererData.MaxTextureSlots; ++i)
			samplers[i] = i;

		s_RendererData.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_RendererData.TextureShader->Bind();
		s_RendererData.TextureShader->SetIntArray("u_Textures", samplers, s_RendererData.MaxTextureSlots);

		// Set all the texture slots to zero
		//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));
		s_RendererData.TextureSlots[0] = s_RendererData.WhiteTexture;

		s_RendererData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_RendererData.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_RendererData.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_RendererData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_RendererStats->Batches = 0;
		s_RendererStats->DrawCalls = 0;
		s_RendererStats->Vertices = 0;
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

		StartBatch();
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

		for (uint32_t i = 0; i < s_RendererData.TextureSlotIndex; ++i)
		{
			s_RendererData.TextureSlots[i]->Bind(i);
		}

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

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
		
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		if (s_RendererData.QuadCount >= s_RendererData.MaxQuads)
			EndBatch();

		const float KTexIndex = 0.0f;
		const float kTilingFactor = 1.0f;

		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quadVertexCount = 4;

		for (uint32_t i = 0; i < quadVertexCount; ++i)
		{
			s_RendererData.QuadVertexBufferPtr->Position = transform * s_RendererData.QuadVertexPositions[i];
			s_RendererData.QuadVertexBufferPtr->Color = color;
			s_RendererData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_RendererData.QuadVertexBufferPtr->TexIndex = KTexIndex;
			s_RendererData.QuadVertexBufferPtr->TilingFactor = kTilingFactor;
			s_RendererData.QuadVertexBufferPtr++;
		}

		s_RendererData.QuadIndexCount += 6;
		s_RendererData.QuadCount++;

		s_RendererStats->DrawCalls++;
		s_RendererStats->Vertices += 4;
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
		
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		if (s_RendererData.QuadCount + 1 > s_RendererData.MaxQuads)
			EndBatch();


		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_RendererData.TextureSlotIndex; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*s_RendererData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_RendererData.TextureSlotIndex;
			s_RendererData.TextureSlots[s_RendererData.TextureSlotIndex++] = texture;
		}

		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quadVertexCount = 4;

		for (uint32_t i = 0; i < quadVertexCount; ++i)
		{
			s_RendererData.QuadVertexBufferPtr->Position = transform * s_RendererData.QuadVertexPositions[i];
			s_RendererData.QuadVertexBufferPtr->Color = tintColor;
			s_RendererData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_RendererData.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_RendererData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_RendererData.QuadVertexBufferPtr++;
		}
		s_RendererData.QuadIndexCount += 6;
		s_RendererData.QuadCount++;

		s_RendererStats->DrawCalls++;
		s_RendererStats->Vertices += 4;
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

	void Renderer2D::StartNewFrameStats()
	{
		s_RendererStats->DrawCalls = 0;
		s_RendererStats->Batches = 0;
		s_RendererStats->Vertices = 0;
	}

	void Renderer2D::StartBatch()
	{
		s_RendererData.QuadCount = 0;
		s_RendererData.QuadIndexCount = 0;
		s_RendererData.QuadVertexBufferPtr = s_RendererData.QuadVertexBufferBase;

		s_RendererData.TextureSlotIndex = 1;

		s_RendererStats->Batches++;
	}

	void Renderer2D::EndBatch()
	{
		Flush();
		StartBatch();
	}

}