#pragma once

#ifndef KABLUNK_MATH_MATH_H
#define KABLUNK_MATH_MATH_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Timestep.h"

namespace kb::math
{ // start namespace kb::math

bool decompose_transform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_scale, glm::vec3& out_rotation);

inline glm::mat4 vk_ortho(
    float left_plane,
    float right_plane,
    float bottom_plane,
    float top_plane,
    float near_plane,
    float far_plane
)
{
    return glm::mat4{
      2.0f / (right_plane - left_plane),
      0.0f,
      0.0f,
      0.0f,

      0.0f,
      2.0f / (bottom_plane - top_plane),
      0.0f,
      0.0f,

      0.0f,
      0.0f,
      1.0f / (near_plane - far_plane),
      0.0f,

      -(right_plane + left_plane) / (right_plane - left_plane),
      -(bottom_plane + top_plane) / (bottom_plane - top_plane),
      near_plane / (near_plane - far_plane),
      1.0f
    };
}

KB_FORCE_INLINE glm::vec3 lerp(const glm::vec3& p_x, const glm::vec3& p_y, float p_time)
{
    return p_x + (p_y - p_x) * p_time;
}

// vec4 { x_top_left, y_top_left, x_bottom_right, y_bottom_right }
KB_FORCE_INLINE auto does_point_overlap_rect(const glm::vec2& p_pos, const glm::vec4& p_rect) noexcept -> bool
{
    return p_pos.x >= p_rect.x && p_pos.y >= p_rect.y && p_pos.x <= p_rect.z && p_pos.y <= p_rect.w;
}

// p_current_velocity is out parameter
KB_FORCE_INLINE auto smooth_damp(
    const f32 p_current,
    const f32 p_target,
    f32& p_current_velocity,
    const f32 p_smooth_time,
    const kb::Timestep p_timestep,
    const f32 p_max_speed = std::numeric_limits<f32>::max()
    ) noexcept -> f32
{
    const auto delta_time = p_timestep.GetMiliseconds();

    // based on Game Programming Gems 4 Chapter 1.10
    const f32 clamped_smooth_time = std::max(p_smooth_time, 0.0001f);
    const f32 max_delta = p_max_speed * clamped_smooth_time;

    const f32 omega = 2.f / clamped_smooth_time;

    const f32 x = omega * delta_time;
    const f32 exp = 1.f / (1.f + x * 0.48f * x * x + 0.235f * x * x * x);
    const f32 delta = glm::clamp(p_current - p_target, -max_delta, max_delta);
    const f32 original_target = p_target;
    const f32 new_target = p_current - delta;

    const f32 temp = (p_current_velocity + omega * delta) * delta_time;
    p_current_velocity = (p_current_velocity - omega * temp) * exp;
    f32 output = new_target + (delta + temp) * exp;

    // prevent overshooting
    if ((original_target - p_current > 0.f) == (output > original_target))
    {
        output = original_target;
        p_current_velocity = (output - original_target) / delta_time;
    }

    return output;
}

// #TODO hand vectorize
KB_FORCE_INLINE auto smooth_damp(
    const glm::vec3& p_current,
    const glm::vec3& p_target,
    glm::vec3& p_current_velocity,
    const f32 p_smooth_time,
    const kb::Timestep p_timestep,
    const f32 p_max_speed = std::numeric_limits<f32>::max()
) noexcept -> glm::vec3
{
    return glm::vec3{
        smooth_damp(
            p_current.x,
            p_target.x,
            p_current_velocity.x,
            p_smooth_time,
            p_timestep,
            p_max_speed
        ),
        smooth_damp(
            p_current.y,
            p_target.y,
            p_current_velocity.y,
            p_smooth_time,
            p_timestep,
            p_max_speed
        ),
        smooth_damp(
            p_current.z,
            p_target.z,
            p_current_velocity.z,
            p_smooth_time,
            p_timestep,
            p_max_speed
        ),
    };
}

} // end namespace kb::math

#endif
