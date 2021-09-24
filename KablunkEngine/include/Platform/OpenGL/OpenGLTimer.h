#ifndef KABLUNK_PLATFORM_OPENGL_TIMER_H
#define KABLUNK_PLATFORM_OPENGL_TIMER_H

#include "Kablunk/Core/Timing.h"

namespace Kablunk
{
	class OpenGLTimer : public Timer
	{
	public:
		OpenGLTimer();
		~OpenGLTimer() {};

		float GetTime() const override;
	};
}

#endif

