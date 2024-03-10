#pragma once

#include "Kablunk/Core/CoreTypes.h"

namespace kb
{ // start namespace kb

// #TODO template
[[nodiscard]] KB_FORCE_INLINE auto left_rotate(u32 p_number, u32 p_shift) noexcept -> u32
{
    return (p_number << p_shift) | ((sizeof(u32) * 8) - p_shift);
}

[[nodiscard]] auto compute_md5_hash(const void* p_buffer, size_t p_size) noexcept -> u128;

} // end namespace kb
