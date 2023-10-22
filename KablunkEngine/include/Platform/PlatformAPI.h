#ifndef KABLUNK_PLATFORM_PLATFORM_API_H
#define KABLUNK_PLATFORM_PLATFORM_API_H

namespace kb
{
	class PlatformAPI
	{
	public:
		enum class PLATFORM
		{
			None = 0,
			Windows = 1, 
			Linux = 2,
			Mac = 3,
			Android = 4,
			iOS = 5
		};
	public:
		static float GetTime();
		static PLATFORM GetPlatform() { return s_Platform; };
	private:
		static PLATFORM s_Platform;
	};
}

#endif
