#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include <glm/glm.hpp>

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"

namespace Kablunk
{
	class Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader> shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitMesh(Ref<Mesh> mesh, glm::mat4 transform);

		static Ref<Texture2D> GetWhiteTexture();

		static Ref<ShaderLibrary> GetShaderLibrary();

		inline static RendererAPI::RenderAPI_t GetAPI() { return RendererAPI::GetAPI(); };
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> m_SceneData;
		
		static Ref<ShaderLibrary> s_shader_library;
	};

	
}

#endif 
