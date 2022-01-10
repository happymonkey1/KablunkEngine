#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{
	Scope<SceneData> Renderer::m_SceneData = CreateScope<SceneData>();
	IntrusiveRef<ShaderLibrary> Renderer::s_shader_library = IntrusiveRef<ShaderLibrary>::Create();
	Ref<FT_Library> Renderer::s_freetype_lib = CreateRef<FT_Library>();

	void Renderer::Init()
	{
		KB_PROFILE_FUNCTION();

		// Renderer initialization
		RenderCommand::Init();

		// Setting up data

		// Uniform buffers
		m_SceneData->camera_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::CameraData), 0);
		m_SceneData->renderer_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::RendererData), 1);
		m_SceneData->point_lights_uniform_buffer = UniformBuffer::Create(sizeof(PointLightsData), 3);

		// Initialize freetype
		//if (FT_Init_FreeType(s_freetype_lib.get()))
		//	KB_CORE_ASSERT(false, "Could not initialize FreeType");
		
		Renderer2D::Init();
	}

	Ref<Texture2D> Renderer::GetWhiteTexture()
	{
		return Renderer2D::GetWhiteTexture();
	}

	IntrusiveRef<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_shader_library;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		auto view_mat = glm::inverse(transform);
		auto view_projection_mat = camera.GetProjection() * glm::inverse(transform);

		m_SceneData->camera_buffer.ViewProjectionMatrix = view_projection_mat;
		m_SceneData->camera_buffer.ProjectionMatrix = camera.GetProjection();
		m_SceneData->camera_buffer.ViewMatrix = view_mat;
		m_SceneData->camera_buffer.CameraPosition = transform[3];
		m_SceneData->camera_uniform_buffer->SetData(&m_SceneData->camera_buffer, sizeof(SceneData::CameraData));
	}

	void Renderer::BeginScene(const EditorCamera& editor_camera)
	{
		m_SceneData->camera_buffer.ViewProjectionMatrix = editor_camera.GetViewProjectionMatrix();
		m_SceneData->camera_buffer.ProjectionMatrix = editor_camera.GetProjection();
		m_SceneData->camera_buffer.ViewMatrix = editor_camera.GetViewMatrix();
		m_SceneData->camera_buffer.CameraPosition = editor_camera.GetTranslation();
		m_SceneData->camera_uniform_buffer->SetData(&m_SceneData->camera_buffer, sizeof(SceneData::CameraData));
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->camera_buffer.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitData(const IntrusiveRef<Shader> shader, const IntrusiveRef<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		if (Renderer::GetAPI() == RendererAPI::RenderAPI_t::OpenGL)
		{
			shader->Bind();
			shader.As<OpenGLShader>()->UploadUniformMat4("u_ViewProjection", m_SceneData->camera_buffer.ViewProjectionMatrix);
			shader.As<OpenGLShader>()->UploadUniformMat4("u_Transform", transform);
		}
		else
		{
			//shader.As<VulkanShader>()->UploadUniformMat4("u_ViewProjection", m_SceneData->camera_buffer.ViewProjectionMatrix);
			//shader.As<VulkanShader>()->UploadUniformMat4("u_Transform", transform);
		}

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMesh(IntrusiveRef<Mesh> mesh, glm::mat4 transform)
	{
		// #FIXME bad
		//auto mesh_shader = mesh->GetMeshData()->GetShader().As<OpenGLShader>();
		mesh->GetMeshData()->GetShader()->Bind();

		m_SceneData->renderer_buffer.Transform = transform;
		m_SceneData->renderer_uniform_buffer->SetData(&m_SceneData->renderer_buffer, sizeof(SceneData::RendererData));

		//mesh_shader->SetMat4("u_Transform", transform);

		mesh->GetVertexArray()->Bind();
		//RenderCommand::SetWireframeMode(true);
		RenderCommand::DrawIndexed(mesh->GetVertexArray());
		//RenderCommand::SetWireframeMode(false);
	}

	void Renderer::SubmitPointLights(std::vector<PointLight>& lights, uint32_t count)
	{
		KB_CORE_ASSERT(lights.size() <= count, "count and light vector sizes are different!");
		KB_CORE_ASSERT(lights.size() < MAX_POINT_LIGHTS, "only {0} concurrent point lights are supported!", MAX_POINT_LIGHTS);
		m_SceneData->plights_buffer.count = count;
		memcpy(m_SceneData->plights_buffer.lights, lights.data(), sizeof(PointLight) * count);

		m_SceneData->point_lights_uniform_buffer->SetData(&m_SceneData->plights_buffer, sizeof(uint32_t) + sizeof(PointLight) * count);
	}
}
