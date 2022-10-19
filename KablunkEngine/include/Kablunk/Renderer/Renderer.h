#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include FT_FREETYPE_H
//#include <ft2build.h>

namespace Kablunk
{
	constexpr uint32_t MAX_POINT_LIGHTS = 16;

	enum RendererPipelineDescriptor
	{
		PHONG_DIFFUSE = 0,
		PBR = 1,

		NONE
	};

	struct RendererOptions
	{
		uint32_t frames_in_flight = 3;
		RendererPipelineDescriptor pipeline = PHONG_DIFFUSE;
	};
	
	// #TODO refactor to use new singleton pattern instead of static functions
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);


		static void RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Pipeline> pipeline);
		static void RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Material> material);
		static void OnShaderReloaded(uint64_t hash);

		static uint32_t GetCurrentFrameIndex();

		static IntrusiveRef<Texture2D> GetWhiteTexture();

		static IntrusiveRef<ShaderLibrary> GetShaderLibrary();
		static IntrusiveRef<Shader> GetShader(const std::string& name);

		static const RendererOptions& GetConfig() { return s_options; }

		static const RendererPipelineDescriptor GetRendererPipeline() { return s_options.pipeline; }
		static void SetRendererPipeline(RendererPipelineDescriptor new_pipeline) { s_options.pipeline = new_pipeline; }

		inline static RendererAPI::RenderAPI_t GetAPI() { return RendererAPI::GetAPI(); };

		// \brief get the viewport's os screen position within the application
		const glm::vec2& get_viewport_pos() const { return m_viewport_pos; }
		// \brief get the viewport's size
		const glm::vec2& get_viewport_size() const { return m_viewport_size; }

		SINGLETON_GET_FUNC(Renderer);
	private:
		inline static RendererOptions s_options = { };
		
		static IntrusiveRef<ShaderLibrary> s_shader_library;

		// store the viewport's os screen position within the application
		// used for calculating screen to world space in the editor
		glm::vec2 m_viewport_pos = glm::vec2{ 0.0f };
		// store the viewport's size
		// used for calculating screen to world space in the editor
		glm::vec2 m_viewport_size = glm::vec2{ 0.0f };

		friend class EditorLayer;
	};

	
}

#endif 
