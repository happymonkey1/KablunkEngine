#ifndef KABLUNK_MATH_MATH_H
#define KABLUNK_MATH_MATH_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace kb::math
{
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
}

#endif
