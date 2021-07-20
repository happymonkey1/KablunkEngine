#ifndef KABLUNK_PCH_H
#define KABLUNK_PCH_H


#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <sstream>

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Log.h"
#include "Kablunk/Debug/Instrumentor.h"
#include "Kablunk/Utilities/StaticReflection.h"

// #TODO Would prefer to include via <stduuid/uuid.h>
#include <uuid.h>

#ifdef KB_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

#endif
