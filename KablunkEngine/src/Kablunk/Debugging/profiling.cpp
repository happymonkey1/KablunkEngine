#include "kablunkpch.h"

#include "Kablunk/Debugging/profiling.h"

#ifdef KB_PROFILE_MEMORY
void* operator new(std::size_t size)
{
    auto ptr = std::malloc(size);
    TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void* ptr)
{
    TracyFree(ptr);
    free(ptr);
}
#endif
