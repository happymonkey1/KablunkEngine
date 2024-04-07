#pragma once

#include "Kablunk/Core/Core.h"

#include <vulkan/vulkan.h>

namespace kb
{ // start namespace kb

#define KB_VK_CHECK_RESULT(x) KB_CORE_ASSERT(((x) == VK_SUCCESS), "[Vulkan]: Vulkan call failed!");

} // end namespace kb
