#pragma once
#include "Kablunk/Asset/Asset.h"

namespace kb::render
{ // start namespace kb::render

class render_resource : public ::kb::asset::Asset
{
public:
    using resource_descriptor_info_t = void*;

public:
    virtual ~render_resource() override = default;
    virtual resource_descriptor_info_t get_descriptor_info() noexcept = 0;
};

} // end namespace kb::render
