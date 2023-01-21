#ifndef KB_OPENGL_RENDERERAPI_H
#define KB_OPENGL_RENDERERAPI_H

#include "Kablunk/Renderer/RendererAPI.h"

#include "Kablunk/Renderer/MaterialAsset.h"

namespace Kablunk
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override {};
		
		virtual void BeginFrame() override {}
		virtual void EndFrame() override {}
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) override { KB_CORE_ASSERT(false, "not implemented!") };
		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void RenderMesh(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void RenderMeshWithMaterial(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

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
		) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}
		virtual void RenderQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBuffer> storage_buffer_set, IntrusiveRef<Material> material, const glm::mat4& transform)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}
		virtual void RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void SetLineWidth(IntrusiveRef<RenderCommandBuffer> render_command_buffer, float line_width) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false) override { KB_CORE_ASSERT(false, "not implemented"); };
		virtual void EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer) override { KB_CORE_ASSERT(false, "not implemented"); }

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

		virtual void WaitAndRender() override {}
	private:
		bool m_draw_wireframe{ false };
	};
	
}


#endif
