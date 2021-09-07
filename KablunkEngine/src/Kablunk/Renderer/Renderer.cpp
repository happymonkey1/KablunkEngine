#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Kablunk
{
	Scope<SceneData> Renderer::m_SceneData = CreateScope<SceneData>();
	Ref<ShaderLibrary> Renderer::s_shader_library = CreateRef<ShaderLibrary>();

	void Renderer::Init()
	{
		KB_PROFILE_FUNCTION();

		// #TODO move elsewhere when refactoring renderer
		s_shader_library->Load("resources/shaders/Kablunk_diffuse_static.glsl");

		// Setting up data

		// Uniform buffers
		m_SceneData->camera_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::CameraData), 0);
		m_SceneData->renderer_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::RendererData), 1);
		m_SceneData->point_lights_uniform_buffer = UniformBuffer::Create(sizeof(uint32_t) + sizeof(PointLightData) * MAX_POINT_LIGHTS, 2);

		RenderCommand::Init();
		Renderer2D::Init();
	}

	Ref<Texture2D> Renderer::GetWhiteTexture()
	{
		return Renderer2D::GetWhiteTexture();
	}

	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
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
		m_SceneData->camera_buffer.InverseViewProjectionMatrix = glm::inverse(view_projection_mat);
		m_SceneData->camera_buffer.ProjectionMatrix = camera.GetProjection();
		m_SceneData->camera_buffer.ViewMatrix = view_mat;
		m_SceneData->camera_buffer.CameraPosition = transform[3];
		m_SceneData->camera_uniform_buffer->SetData(&m_SceneData->camera_buffer, sizeof(SceneData::CameraData));
	}

	void Renderer::BeginScene(const EditorCamera& editor_camera)
	{
		m_SceneData->camera_buffer.ViewProjectionMatrix = editor_camera.GetViewProjectionMatrix();
		m_SceneData->camera_buffer.InverseViewProjectionMatrix = glm::inverse(editor_camera.GetViewProjectionMatrix());
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

	void Renderer::Submit(const Ref<Shader> shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->camera_buffer.ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMesh(Ref<Mesh> mesh, glm::mat4 transform)
	{
		// #FIXME bad
		auto mesh_shader = std::dynamic_pointer_cast<OpenGLShader>(mesh->GetMeshData()->GetShader());
		mesh_shader->Bind();

		m_SceneData->renderer_buffer.Transform = transform;
		m_SceneData->renderer_uniform_buffer->SetData(&m_SceneData->renderer_buffer, sizeof(SceneData::RendererData));

		//mesh_shader->SetMat4("u_Transform", transform);

		mesh->GetVertexArray()->Bind();
		//RenderCommand::SetWireframeMode(true);
		RenderCommand::DrawIndexed(mesh->GetVertexArray());
		//RenderCommand::SetWireframeMode(false);
	}

	void Renderer::SubmitPointLights(std::vector<PointLightData>& lights, uint32_t count)
	{
		KB_CORE_ASSERT(lights.size() < MAX_POINT_LIGHTS, "only {0} concurrent point lights are supported!", MAX_POINT_LIGHTS);
		m_SceneData->plights_buffer = { count, lights.data() };
		
		m_SceneData->point_lights_uniform_buffer->SetData(&m_SceneData->plights_buffer, sizeof(uint32_t) + sizeof(PointLightData) * count);
	}
}
