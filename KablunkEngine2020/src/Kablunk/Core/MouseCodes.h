#pragma once

namespace Kablunk
{
	using MouseCode = uint16_t;

	namespace Mouse
	{
		// FROM glfw3.h
		enum : MouseCode
		{
			Button1		= 0,
			Button2		= 1,
			Button3		= 2,
			Button4		= 3,
			Button5		= 4,
			Button6		= 5,
			Button7		= 6,
			Button8		= 7,

			Left		= Button1,
			Right		= Button2,
			Middle		= Button3,
			Last		= Button8
		};
	}
}