#ifndef KABLUNK_RENDERER_GRAPHICS_CONTEXT_H
#define KABLUNK_RENDERER_GRAPHICS_CONTEXT_H


namespace Kablunk 
{
	class GraphicsContext 
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void Shutdown() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}

#endif
