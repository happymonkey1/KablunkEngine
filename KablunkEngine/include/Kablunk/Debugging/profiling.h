#pragma once

#define KB_ENABLE_PROFILING !defined(KB_DISTRIBUTION)

#if KB_ENABLE_PROFILING
#   include <optick.h>
#endif

#if KB_ENABLE_PROFILING
#   define KB_PROFILE_FRAME(...) OPTICK_FRAME(__VA_ARGS__)
#   define KB_PROFILE_FUNC(...) OPTICK_EVENT(__VA_ARGS__)
#   define KB_PROFILE_TAG(NAME, ...) OPTICK_TAG(NAME, __VA_ARGS__)
#   define KB_PROFILE_SCOPE_DYNAMIC(NAME) OPTICK_EVENT_DYNAMIC(NAME)
#   define KB_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#else
#   define KB_PROFILE_FRAME(...)
#   define KB_PROFILE_FUNC(...)
#   define KB_PROFILE_TAG(NAME, ...)
#   define KB_PROFILE_SCOPE_DYNAMIC(NAME)
#   define KB_PROFILE_THREAD(...)
#endif
