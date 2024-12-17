require "vector2"
require "vector3"
require "vector4"

logger_info("Hello from lua!")

g_renderer_2d.draw_quad(
    Vector3(0, 0, 0),
    Vector2(0, 0),
    "kb::texture::white_texture",
    Vector4(1.0, 1.0, 1.0, 1.0)
)

g_renderer_2d.draw_quad(
    Vector3(1, 0, 0),
    Vector2(0, 0),
    "kb::texture::white_texture",
    Vector4(1.0, 0.0, 0.0, 1.0)
)

g_renderer_2d.draw_quad(
    Vector3(0, 1, 0),
    Vector2(0, 0),
    "kb::texture::white_texture",
    Vector4(0.0, 1.0, 0.0, 1.0)
)

g_renderer_2d.draw_quad(
    Vector3(1, 1, 0),
    Vector2(0, 0),
    "kb::texture::white_texture",
    Vector4(0.0, 0.0, 1.0, 1.0)
)
