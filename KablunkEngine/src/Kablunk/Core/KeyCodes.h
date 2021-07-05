#pragma once

namespace Kablunk
{
	using KeyCode = uint16_t;

	namespace Key
	{
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
			Grave			= 96

			// #TODO do rest of keycodes you lazy ass
		};

	}
}



#define KB_KEY_WORLD_1            161 /* non-US #1 */
#define KB_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define KB_KEY_ESCAPE             256
#define KB_KEY_ENTER              257
#define KB_KEY_TAB                258
#define KB_KEY_BACKSPACE          259
#define KB_KEY_INSERT             260
#define KB_KEY_DELETE             261
#define KB_KEY_RIGHT              262
#define KB_KEY_LEFT               263
#define KB_KEY_DOWN               264
#define KB_KEY_UP                 265
#define KB_KEY_PAGE_UP            266
#define KB_KEY_PAGE_DOWN          267
#define KB_KEY_HOME               268
#define KB_KEY_END                269
#define KB_KEY_CAPS_LOCK          280
#define KB_KEY_SCROLL_LOCK        281
#define KB_KEY_NUM_LOCK           282
#define KB_KEY_PRINT_SCREEN       283
#define KB_KEY_PAUSE              284
#define KB_KEY_F1                 290
#define KB_KEY_F2                 291
#define KB_KEY_F3                 292
#define KB_KEY_F4                 293
#define KB_KEY_F5                 294
#define KB_KEY_F6                 295
#define KB_KEY_F7                 296
#define KB_KEY_F8                 297
#define KB_KEY_F9                 298
#define KB_KEY_F10                299
#define KB_KEY_F11                300
#define KB_KEY_F12                301
#define KB_KEY_F13                302
#define KB_KEY_F14                303
#define KB_KEY_F15                304
#define KB_KEY_F16                305
#define KB_KEY_F17                306
#define KB_KEY_F18                307
#define KB_KEY_F19                308
#define KB_KEY_F20                309
#define KB_KEY_F21                310
#define KB_KEY_F22                311
#define KB_KEY_F23                312
#define KB_KEY_F24                313
#define KB_KEY_F25                314
#define KB_KEY_KP_0               320
#define KB_KEY_KP_1               321
#define KB_KEY_KP_2               322
#define KB_KEY_KP_3               323
#define KB_KEY_KP_4               324
#define KB_KEY_KP_5               325
#define KB_KEY_KP_6               326
#define KB_KEY_KP_7               327
#define KB_KEY_KP_8               328
#define KB_KEY_KP_9               329
#define KB_KEY_KP_DECIMAL         330
#define KB_KEY_KP_DIVIDE          331
#define KB_KEY_KP_MULTIPLY        332
#define KB_KEY_KP_SUBTRACT        333
#define KB_KEY_KP_ADD             334
#define KB_KEY_KP_ENTER           335
#define KB_KEY_KP_EQUAL           336
#define KB_KEY_LEFT_SHIFT         340
#define KB_KEY_LEFT_CONTROL       341
#define KB_KEY_LEFT_ALT           342
#define KB_KEY_LEFT_SUPER         343
#define KB_KEY_RIGHT_SHIFT        344
#define KB_KEY_RIGHT_CONTROL      345
#define KB_KEY_RIGHT_ALT          346
#define KB_KEY_RIGHT_SUPER        347
#define KB_KEY_MENU               348

#define KB_KEY_LAST               KB_KEY_MENU