#pragma once

namespace kb
{
	using MouseCode = uint16_t;

	enum class MouseMotionMode : uint8_t
	{
		Raw = 0,
		Normal = 1
	};

	namespace Mouse
	{
		// FROM glfw3.h
		enum : MouseCode
		{
			Button0			= 0,
			Button1			= 1,
			Button2			= 2,
			Button3			= 3,
			Button4			= 4,
			Button5			= 5,
			Button6			= 6,
			Button7			= 7,

			ButtonLeft		= Button0,
			ButtonRight		= Button1,
			ButtonMiddle	= Button2,
			ButtonLast		= Button7
		};
	}
}
