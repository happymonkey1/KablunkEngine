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
	// forward declaration
	class RendererAPI;

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
	
	// #TODO refactor rendererapi (remove) by moving into renderer
	class Renderer
	{
	public:
		void init();
		void shutdown();

		void RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Pipeline> pipeline);
		void RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Material> material);
		void OnShaderReloaded(uint64_t hash);

		uint32_t GetCurrentFrameIndex();

		IntrusiveRef<Texture2D> GetWhiteTexture();

		IntrusiveRef<ShaderLibrary> GetShaderLibrary();
		IntrusiveRef<Shader> GetShader(const std::string& name);

		const RendererOptions& GetConfig() { return m_options; }

		const RendererPipelineDescriptor GetRendererPipeline() { return m_options.pipeline; }
		void SetRendererPipeline(RendererPipelineDescriptor new_pipeline) { m_options.pipeline = new_pipeline; }

		static RendererAPI::render_api_t GetAPI() { return RendererAPI::GetAPI(); };

		// \brief get the viewport's os screen position within the application
		const glm::vec2& get_viewport_pos() const { return m_viewport_pos; }
		// \brief get the viewport's size
		const glm::vec2& get_viewport_size() const { return m_viewport_size; }

		// #TODO remove when rendererapi is refactored
		RendererAPI* get_renderer() { KB_CORE_ASSERT(m_renderer_api, "RendererAPI not set?"); return m_renderer_api; }

		SINGLETON_GET_FUNC(Renderer);
	private:
		struct ShaderDependencies
		{
			std::vector<IntrusiveRef<Pipeline>> pipelines;
			std::vector<IntrusiveRef<Material>> materials;
		};

		std::unordered_map<uint64_t, ShaderDependencies> m_shader_dependencies;
		RendererOptions m_options = { };
		IntrusiveRef<ShaderLibrary> m_shader_library;
		RendererAPI* m_renderer_api = nullptr;

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
