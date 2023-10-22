#ifndef KB_OPENGL_RENDERERAPI_H
#define KB_OPENGL_RENDERERAPI_H

#include "Kablunk/Renderer/RendererAPI.h"

#include "Kablunk/Renderer/MaterialAsset.h"

namespace kb
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

		virtual void ClearImage(ref<RenderCommandBuffer> commandBuffer, ref<Image2D> image) override { KB_CORE_ASSERT(false, "not implemented!") };
		virtual void DrawIndexed(const ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void RenderMesh(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<MaterialTable> material_table, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void RenderMeshWithMaterial(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<Material> material, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

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
		) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void SubmitFullscreenQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}
		virtual void RenderQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, const glm::mat4& transform)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}
		virtual void RenderGeometry(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, ref<VertexBuffer> vertex_buffer, ref<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void SetLineWidth(ref<RenderCommandBuffer> render_command_buffer, float line_width) override
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		virtual void BeginRenderPass(ref<RenderCommandBuffer> render_command_buffer, const ref<RenderPass>& render_pass, bool explicit_clear = false) override { KB_CORE_ASSERT(false, "not implemented"); };
		virtual void EndRenderPass(ref<RenderCommandBuffer> render_command_buffer) override { KB_CORE_ASSERT(false, "not implemented"); }

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

		virtual void WaitAndRender() override {}
	private:
		bool m_draw_wireframe{ false };
	};
	
}


#endif
