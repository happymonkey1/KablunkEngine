#ifndef KABLUNK_PLATFORM_OPENGL_TIMER_H
#define KABLUNK_PLATFORM_OPENGL_TIMER_H

#include "Kablunk/Core/Timer.h"

namespace Kablunk
{
	class KABLUNK_API OpenGLTimer : public Timer
	{
	public:
		OpenGLTimer();
		~OpenGLTimer() {};

		float GetTime() const override;
	};
}

#endif

