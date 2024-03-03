#pragma once

#include "Kablunk/Core/Core.h"

namespace kb
{ // start namespace kb

struct lua_error
{
    i32 m_error_code = 0;
    char* m_error_msg = nullptr;
};

} // end namespace kb
