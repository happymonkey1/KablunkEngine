#ifndef RENDERER_H
#define RENDERER_H
#include "RenderCommand.h"


namespace Kablunk
{
	

	

	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		using RendererID = uint32_t;
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); };
	private:

	};

	
}

#endif 
