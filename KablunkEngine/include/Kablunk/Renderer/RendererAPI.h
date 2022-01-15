#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"
#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/Material.h"

#include <glm/glm.hpp>

namespace Kablunk 
{
	class RendererAPI : public RefCounted
	{
	public:
		enum class RenderAPI_t
		{
			None = 0, OpenGL = 1, Vulkan = 2
		};

	public:
		virtual ~RendererAPI() = default;
		
		virtual void Init() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) = 0;
		virtual void SetWireframeMode(bool draw_wireframe) = 0;

		virtual void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material) = 0;


		virtual void WaitAndRender() = 0;

		static inline RenderAPI_t GetAPI() { return s_API; };
	private:
		inline static RenderAPI_t s_API = RenderAPI_t::Vulkan;
	};

}
