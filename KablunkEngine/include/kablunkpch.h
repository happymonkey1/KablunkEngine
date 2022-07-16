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
#include "Kablunk/Core/Logger.h"
#include "Kablunk/Events/Event.h"
#include "Kablunk/Debugging/Instrumentor.h"
#include "Kablunk/Utilities/Reflection/ReflectionCore.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Singleton.h"

#ifdef KB_PLATFORM_WINDOWS
#	include <Windows.h>
#endif

#endif
