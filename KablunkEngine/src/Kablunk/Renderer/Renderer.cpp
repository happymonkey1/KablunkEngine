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
		s_shader_library->Load("resources/shaders/KablunkPBR_static.glsl");

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
		m_SceneData->ViewProjectionMatrix = view_projection_mat;
		m_SceneData->InverseViewProjectionMatrix = glm::inverse(view_projection_mat);
		m_SceneData->ProjectionMatrix = camera.GetProjection();
		m_SceneData->ViewMatrix = view_mat;
	}

	void Renderer::BeginScene(const EditorCamera& editor_camera)
	{
		m_SceneData->ViewProjectionMatrix = editor_camera.GetViewProjectionMatrix();
		m_SceneData->InverseViewProjectionMatrix = glm::inverse(editor_camera.GetViewProjectionMatrix());
		m_SceneData->ProjectionMatrix = editor_camera.GetProjection();
		m_SceneData->ViewMatrix = editor_camera.GetViewMatrix();
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const Ref<Shader> shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMesh(Ref<Mesh> mesh, glm::mat4 transform)
	{
		// #FIXME bad
		auto mesh_shader = std::dynamic_pointer_cast<OpenGLShader>(mesh->GetMeshData()->GetShader());
		mesh_shader->Bind();

		mesh_shader->UploadUniformMat4("Transform", transform);
		mesh_shader->UploadUniformMat4("u_ViewProjectionMatrix", m_SceneData->ViewProjectionMatrix);
		mesh_shader->UploadUniformMat4("u_InverseViewProjectionMatrix", m_SceneData->InverseViewProjectionMatrix);
		mesh_shader->UploadUniformMat4("u_ProjectionMatrix", m_SceneData->ProjectionMatrix);
		mesh_shader->UploadUniformMat4("u_ViewMatrix", m_SceneData->ViewMatrix);


		mesh->GetVertexArray()->Bind();
		RenderCommand::DrawIndexed(mesh->GetVertexArray());
	}
}
