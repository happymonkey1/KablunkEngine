#pragma once

#include "Kablunk/Core/Core.h"

namespace kb
{ // start namespace kb

enum class lua_error_t
{
    internal_engine_error,
    script_not_found,
    script_error,
};

} // end namespace kb
