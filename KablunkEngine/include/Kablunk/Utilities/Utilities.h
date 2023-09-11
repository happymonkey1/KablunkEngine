#ifndef KABLUNK_UTILITIES_UTILITIES_H
#define KABLUNK_UTILITIES_UTILITIES_H

#include <cstdlib>
#include <string>

namespace kb::Utils
{
	template <typename T>
	static void WaitForSecondsCallback(float delay, T callback);
}

#endif
