#ifndef KABLUNK_PLATFORM_WINDOWS_TIMER_H
#define KABLUNK_PLATFORM_WINDOWS_TIMER_H

#include "Kablunk/Core/Timer.h"

namespace Kablunk
{
	class WindowsTimer : public Timer
	{
		WindowsTimer();
		~WindowsTimer() {};

		float GetTime() const override;

	};
}

#endif
