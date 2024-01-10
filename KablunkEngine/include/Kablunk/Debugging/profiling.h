#pragma once

#ifdef TRACY_ENABLE
#   define KB_ENABLE_PROFILING !defined(KB_DISTRIBUTION)
#   include <tracy/Tracy.hpp>

#define KB_PROFILE_MEMORY

#ifdef KB_PROFILE_MEMORY

void* operator new(std::size_t size);

void operator delete(void* ptr) noexcept;


#endif

#endif

#if KB_ENABLE_PROFILING
#   define KB_FRAME_MARK FrameMark
#   define KB_PROFILE_SCOPE ZoneScoped
#   define KB_PROFILE_SCOPE_NAMED(name) ZoneScopedN(name)
#else
#   define KB_FRAME_MARK
#   define KB_PROFILE_SCOPE
#   define KB_PROFILE_SCOPE_NAMED(name)
#endif
