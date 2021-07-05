#ifndef KABLUNK_CORE_TIMER_H
#define KABLUNK_CORE_TIMER_H

#include "Kablunk.h"

namespace Kablunk
{
	class Timer
	{
	public:
		virtual ~Timer() {};
		virtual float GetTime() const = 0;
	};

	
}

#endif // 
