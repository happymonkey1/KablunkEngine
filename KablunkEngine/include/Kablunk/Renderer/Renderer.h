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
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/Material.h"


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
	private:
		inline static RendererOptions s_options = { };
		
		static IntrusiveRef<ShaderLibrary> s_shader_library;

		
	};

	
}

#endif 
