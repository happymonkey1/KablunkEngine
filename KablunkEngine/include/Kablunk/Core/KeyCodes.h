#pragma once

namespace kb
{
	using KeyCode = uint16_t;

	// #TODO probably should be moved elsewhere
	enum class CursorMode : uint8_t
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	namespace Key
	{

		inline bool is_valid_key(KeyCode key)
		{
			switch (key)
			{
				case 32: return true;
				case 39: return true;
				case 44: return true;
				case 45: return true;
				case 46: return true;
				case 47: return true;
				case 48: return true;
				case 49: return true;
				case 50: return true;
				case 51: return true;
				case 52: return true;
				case 53: return true;
				case 54: return true;
				case 55: return true;
				case 56: return true;
				case 57: return true;
				case 59: return true;
				case 61: return true;
				case 65: return true;
				case 66: return true;
				case 67: return true;
				case 68: return true;
				case 69: return true;
				case 70: return true;
				case 71: return true;
				case 72: return true;
				case 73: return true;
				case 74: return true;
				case 75: return true;
				case 76: return true;
				case 77: return true;
				case 78: return true;
				case 79: return true;
				case 80: return true;
				case 81: return true;
				case 82: return true;
				case 83: return true;
				case 84: return true;
				case 85: return true;
				case 86: return true;
				case 87: return true;
				case 88: return true;
				case 89: return true;
				case 90: return true;
				case 91: return true;
				case 93: return true;
				case 92: return true;
				case 96: return true;
				case 161: return true;
				case 162: return true;
				case 256: return true;
				case 257: return true;
				case 258: return true;
				case 259: return true;
				case 280: return true;
				case 262: return true;
				case 263: return true;
				case 264: return true;
				case 265: return true;
				case 260: return true;
				case 261: return true;
				case 266: return true;
				case 267: return true;
				case 268: return true;
				case 269: return true;
				case 283: return true;
				case 281: return true;
				case 284: return true;
				case 290: return true;
				case 291: return true;
				case 292: return true;
				case 293: return true;
				case 294: return true;
				case 295: return true;
				case 296: return true;
				case 297: return true;
				case 298: return true;
				case 299: return true;
				case 300: return true;
				case 301: return true;
				case 302: return true;
				case 303: return true;
				case 304: return true;
				case 305: return true;
				case 306: return true;
				case 307: return true;
				case 308: return true;
				case 309: return true;
				case 310: return true;
				case 311: return true;
				case 312: return true;
				case 313: return true;
				case 314: return true;
				case 282: return true;
				case 320: return true;
				case 321: return true;
				case 322: return true;
				case 323: return true;
				case 324: return true;
				case 325: return true;
				case 326: return true;
				case 327: return true;
				case 328: return true;
				case 329: return true;
				case 330: return true;
				case 331: return true;
				case 332: return true;
				case 333: return true;
				case 334: return true;
				case 335: return true;
				case 336: return true;
				case 340: return true;
				case 341: return true;
				case 342: return true;
				case 343: return true;
				case 344: return true;
				case 345: return true;
				case 346: return true;
				case 347: return true;
				case 348: return true;
				default: return false;
			}
		}

		enum : KeyCode
		{
			//FROM glfw3.h
			Space			= 32,
			Apostrophe		= 39,
			Comma			= 44,
			Minus			= 45,
			Period			= 46,
			Slash			= 47,

			Zero			= 48,
			One				= 49,
			Two				= 50,
			Three			= 51,
			Four			= 52,
			Five			= 53,
			Six				= 54,
			Seven			= 55,
			Eight			= 56,
			Nine			= 57,

			Semicolon		= 59,
			Equal			= 61,

			A				= 65,
			B				= 66,
			C				= 67,
			D				= 68, 
			E				= 69,
			F				= 70,
			G				= 71,
			H				= 72,
			I				= 73,
			J				= 74,
			K				= 75,
			L				= 76,
			M				= 77,
			N				= 78,
			O				= 79,
			P				= 80,
			Q				= 81,
			R				= 82,
			S				= 83,
			T				= 84,
			U				= 85,
			V				= 86,
			W				= 87,
			X				= 88,
			Y				= 89,
			Z				= 90,

			LeftBracket		= 91,
			RightBracket	= 93,
			Backslash		= 92,
			Grave			= 96,

			World1 = 161, /* non-US #1 */
			World2 = 162, /* non-US #2 */

			/* Function keys */
			Escape = 256,
			Enter = 257,
			Tab = 258,
			Backspace = 259,
			CapsLock = 280,
			

			Right = 262,
			Left = 263,
			Down = 264,
			Up = 265,

			Insert = 260,
			Delete = 261,
			PageUp = 266,
			PageDown = 267,
			Home = 268,
			End = 269,
			
			PrintScreen = 283,
			ScrollLock = 281,
			Pause = 284,

			F1 = 290,
			F2 = 291,
			F3 = 292,
			F4 = 293,
			F5 = 294,
			F6 = 295,
			F7 = 296,
			F8 = 297,
			F9 = 298,
			F10 = 299,
			F11 = 300,
			F12 = 301,
			F13 = 302,
			F14 = 303,
			F15 = 304,
			F16 = 305,
			F17 = 306,
			F18 = 307,
			F19 = 308,
			F20 = 309,
			F21 = 310,
			F22 = 311,
			F23 = 312,
			F24 = 313,
			F25 = 314,

			NumLock = 282,
			KP0 = 320,
			KP1 = 321,
			KP2 = 322,
			KP3 = 323,
			KP4 = 324,
			KP5 = 325,
			KP6 = 326,
			KP7 = 327,
			KP8 = 328,
			KP9 = 329,

			KPDecimal = 330,
			KPDivide = 331,
			KPMultiply = 332,
			KPSubtract = 333,
			KPAdd = 334,
			KPEnter = 335,
			KPEqual = 336,

			LeftShift = 340,
			LeftControl = 341,
			LeftAlt = 342,
			LeftSuper = 343,
			RightShift = 344,
			RightControl = 345,
			RightAlt = 346,
			RightSuper = 347,
			Menu = 348
		};

	}
}
