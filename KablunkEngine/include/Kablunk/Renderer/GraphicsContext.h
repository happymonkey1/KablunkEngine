#ifndef KABLUNK_RENDERER_GRAPHICS_CONTEXT_H
#define KABLUNK_RENDERER_GRAPHICS_CONTEXT_H


namespace Kablunk 
{
	class GraphicsContext : public RefCounted
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void Shutdown() = 0;

		static ref<GraphicsContext> Create(void* window);
	};
}

#endif
