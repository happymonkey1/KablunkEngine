#pragma once

#include <msdf-atlas-gen.h>
#include <filesystem>

namespace kb::render
{ // start namespace kb::render

auto generate_atlas(const std::filesystem::path& p_filepath) noexcept -> bool;

} // end namespace kb::render
