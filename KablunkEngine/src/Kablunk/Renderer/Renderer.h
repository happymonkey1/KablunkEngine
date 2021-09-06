#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include <glm/glm.hpp>

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/UniformBuffer.h"


namespace Kablunk
{
	struct SceneData
	{
		struct CameraData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 InverseViewProjectionMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::vec3 CameraPosition;
		};
		
		struct RendererData
		{
			glm::mat4 Transform;
		};

		CameraData camera_buffer;
		RendererData renderer_buffer;
		Ref<UniformBuffer> camera_uniform_buffer;
		Ref<UniformBuffer> renderer_uniform_buffer;
	};

	class Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		// #TODO remove
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();


		static void Submit(const Ref<Shader> shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitMesh(Ref<Mesh> mesh, glm::mat4 transform);

		static Ref<Texture2D> GetWhiteTexture();

		static Ref<ShaderLibrary> GetShaderLibrary();

		inline static RendererAPI::RenderAPI_t GetAPI() { return RendererAPI::GetAPI(); };
	private:
		static Scope<SceneData> m_SceneData;
		
		static Ref<ShaderLibrary> s_shader_library;
	};

	
}

#endif 
