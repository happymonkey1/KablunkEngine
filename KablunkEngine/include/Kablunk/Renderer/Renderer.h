#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/UniformBuffer.h"

#include <ft2build.h>
#include FT_FREETYPE_H


namespace Kablunk
{
	constexpr uint32_t MAX_POINT_LIGHTS = 16;

	// #TODO move to scene
	struct PointLight
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		float Multiplier = { 1.0f };
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Radius = { 10.0f };
		float Min_radius = { 1.0f };
		float Falloff = { 1.0f };

		char Padding[8]{}; // Don't know why this is needed, but sizeof PointLight = 48 bytes makes it work???
	};

	// #TODO rename to avoid confusion
	struct PointLightsData
	{
		uint32_t count;
		glm::vec3 Padding{}; // I have no idea why this is needed but it is
		PointLight lights[MAX_POINT_LIGHTS]{};
	};

	struct SceneData
	{
		struct CameraData
		{
			glm::mat4 ViewProjectionMatrix;
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
		PointLightsData plights_buffer;
		IntrusiveRef<UniformBuffer> camera_uniform_buffer;
		IntrusiveRef<UniformBuffer> renderer_uniform_buffer;
		IntrusiveRef<UniformBuffer> point_lights_uniform_buffer;
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

		

		static void SubmitData(const IntrusiveRef<Shader> shader, const IntrusiveRef<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitMesh(IntrusiveRef<Mesh> mesh, glm::mat4 transform);
		static void SubmitPointLights(std::vector<PointLight>& data, uint32_t count);

		static uint32_t GetCurrentFrameIndex();

		static IntrusiveRef<Texture2D> GetWhiteTexture();

		static IntrusiveRef<ShaderLibrary> GetShaderLibrary();

		inline static RendererAPI::RenderAPI_t GetAPI() { return RendererAPI::GetAPI(); };
	private:
		static Scope<SceneData> m_SceneData;
		
		static IntrusiveRef<ShaderLibrary> s_shader_library;

		static Ref<FT_Library> s_freetype_lib;

		
	};

	
}

#endif 
