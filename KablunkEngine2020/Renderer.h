#ifndef RENDERER_H
#define RENDERER_H

namespace kablunk
{
	enum class RendererAPI
	{
		None = 0, OpenGL = 1
	};

	

	class Renderer
	{
	public:
		using RendererID = uint32_t;
		inline static RendererAPI GetAPI() { return s_RendererAPI; };
	private:
		static RendererAPI s_RendererAPI;
	};

	
}

#endif // 
