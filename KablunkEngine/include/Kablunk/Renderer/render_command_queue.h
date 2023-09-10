#pragma once
#ifndef KABLUNK_RENDERER_RENDER_COMMAND_QUEUE_H
#define KABLUNK_RENDERER_RENDER_COMMAND_QUEUE_H

#include <stdint.h>

namespace kb
{
	class render_command_queue
	{
	public:
		//using RenderCommandFn = void(*)(void*);
		typedef void(*RenderCommandFn)(void*);

        // default constructor
		render_command_queue();
        // no copy constructor
        render_command_queue(const render_command_queue& p_other) = delete;
        // move constructor
        render_command_queue(render_command_queue&& p_other) noexcept;
        // destructor
		~render_command_queue();
        
        // allocate a render command in the buffer
		void* allocate(RenderCommandFn func, uint32_t size);
        // execute all allocated commands
		void execute();

        // check whether the command queue has any commands inside the buffer
        bool is_empty() const { return m_command_buffer == m_command_buffer_ptr; }
        // return the number of commands recorded to the buffer
        uint32_t get_command_count() const { return m_command_count; }

        // move assign operator
        render_command_queue& operator=(render_command_queue&& p_other) noexcept;
        // no copy assign operator
        render_command_queue& operator=(const render_command_queue& p_other) = delete;
	private:
        // pointer to the head of the buffer
		uint8_t* m_command_buffer;
        // pointer to the end of the buffer
		uint8_t* m_command_buffer_ptr;
        // number of commands allocated on the buffer
		uint32_t m_command_count;
	};
}

#endif
