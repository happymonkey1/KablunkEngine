#pragma once

#define KB_ENABLE_PROFILING !defined(KB_DISTRIBUTION)

#if KB_ENABLE_PROFILING
#    define TRACY_ENABLE
#    include <tracy/Tracy.hpp>
#endif

#if KB_ENABLE_PROFILING
#   define KB_FRAME_MARK FrameMark
#   define KB_PROFILE_SCOPE ZoneScoped
#   define KB_PROFILE_SCOPE_NAMED(name) ZoneScopedN(name)
#else
#   define KB_FRAME_MARK(...)
#   define KB_PROFILE_FUNC(...)
#   define KB_PROFILE_TAG(NAME, ...)
#   define KB_PROFILE_SCOPE_DYNAMIC(NAME)
#   define KB_PROFILE_THREAD(...)
#endif
