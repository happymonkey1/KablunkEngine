#pragma once
#include "Kablunk/Renderer/render_pass.h"

namespace kb::render
{ // start namespace kb::render

class render_graph
{
public:
    // using execute_callback_t = 
public:
    render_graph() noexcept = default;
    ~render_graph() noexcept = default;

    render_graph(const render_graph&) noexcept = delete;
    render_graph(render_graph&&) noexcept = delete;

    auto add_render_pass(
        const ref<render_pass>& p_render_pass
    ) noexcept -> render_graph&;

    // build the render graph, filtering out nodes that do not contribute, then call render callbacks
    auto bake() noexcept -> void;

    auto operator=(const render_graph&) noexcept = delete;
    auto operator=(render_graph&&) noexcept = delete;

private:

private:

};

} // end namespace kb::render
