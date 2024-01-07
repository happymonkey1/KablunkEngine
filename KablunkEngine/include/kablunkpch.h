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

// c++20 includes
#if _MSC_VER >= 1928
#	include <ranges>
#endif

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Logger.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Core/ScopedTimer.h"

#include "Kablunk/Events/Event.h"

#include "Kablunk/Debugging/Instrumentor.h"
#include "Kablunk/Debugging/profiling.h"

#include "Kablunk/Utilities/Reflection/ReflectionCore.h"
#include "Kablunk/Utilities/Containers/FlatHashMap.hpp"

#ifdef KB_PLATFORM_WINDOWS
#	include <windows.h>
#endif

#endif
