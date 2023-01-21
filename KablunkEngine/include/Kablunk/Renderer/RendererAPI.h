#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"
#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RenderPass.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/MaterialAsset.h"

#include <glm/glm.hpp>

namespace Kablunk 
{
	class RendererAPI : public RefCounted
	{
	public:
		enum class render_api_t
		{
			None = 0, OpenGL = 1, Vulkan = 2
		};

	public:
		virtual ~RendererAPI() = default;
		
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) = 0;
		virtual void SetWireframeMode(bool draw_wireframe) = 0;

		// ======
		// Meshes
		// ======

		virtual void RenderMesh(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) = 0;

		virtual void RenderMeshWithMaterial(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms) = 0;

		virtual void render_instanced_submesh(
			IntrusiveRef<RenderCommandBuffer> render_command_buffer,
			IntrusiveRef<Pipeline> pipeline,
			IntrusiveRef<UniformBufferSet> uniform_buffer_set,
			IntrusiveRef<StorageBufferSet> storage_buffer_set,
			IntrusiveRef<Mesh> mesh,
			uint32_t submesh_index,
			IntrusiveRef<MaterialTable> material_table,
			IntrusiveRef<VertexBuffer> transform_buffer,
			uint32_t transform_offset,
			uint32_t instance_count
		) = 0;

		virtual void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material) = 0;
		
		virtual void RenderQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBuffer> storage_buffer_set, IntrusiveRef<Material> material, const glm::mat4& transform) = 0;
		virtual void RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) = 0;

		virtual void SetLineWidth(IntrusiveRef<RenderCommandBuffer> render_command_buffer, float line_width) = 0;

		virtual void BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false) = 0;
		virtual void EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer) = 0;

		virtual void WaitAndRender() = 0;

		static inline render_api_t GetAPI() { return s_API; };
	private:
		inline static render_api_t s_API = render_api_t::Vulkan;
	};

}
