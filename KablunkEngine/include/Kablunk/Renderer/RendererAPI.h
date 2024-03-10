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

namespace kb 
{
	class RendererAPI : public RefCounted
	{
	public:
		enum class render_api_t
		{
			None = 0,
		    Vulkan = 1
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

		virtual void DrawIndexed(const ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void ClearImage(ref<RenderCommandBuffer> commandBuffer, ref<Image2D> image) = 0;
		virtual void SetWireframeMode(bool draw_wireframe) = 0;

		// ======
		// Meshes
		// ======

		virtual void RenderMesh(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<MaterialTable> material_table, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) = 0;

		virtual void RenderMeshWithMaterial(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<Material> material, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, owning_buffer additional_uniforms) = 0;

		virtual void render_instanced_submesh(
			ref<RenderCommandBuffer> render_command_buffer,
			ref<Pipeline> pipeline,
			ref<UniformBufferSet> uniform_buffer_set,
			ref<StorageBufferSet> storage_buffer_set,
			ref<Mesh> mesh,
			uint32_t submesh_index,
			ref<MaterialTable> material_table,
			ref<VertexBuffer> transform_buffer,
			uint32_t transform_offset,
			uint32_t instance_count
		) = 0;

		virtual void SubmitFullscreenQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material) = 0;
		
		virtual void RenderQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, const glm::mat4& transform) = 0;
		virtual void RenderGeometry(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, ref<VertexBuffer> vertex_buffer, ref<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) = 0;

		virtual void SetLineWidth(ref<RenderCommandBuffer> render_command_buffer, float line_width) = 0;

		virtual void BeginRenderPass(ref<RenderCommandBuffer> render_command_buffer, const ref<RenderPass>& render_pass, bool explicit_clear = false) = 0;
		virtual void EndRenderPass(ref<RenderCommandBuffer> render_command_buffer) = 0;

		virtual void WaitAndRender() = 0;

		static inline render_api_t GetAPI() { return s_API; };
	private:
		inline static render_api_t s_API = render_api_t::Vulkan;
	};

}
