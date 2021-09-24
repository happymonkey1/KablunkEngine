#ifndef KABLUNK_MATH_MATH_H
#define KABLUNK_MATH_MATH_H

#include <glm/glm.hpp>

namespace Kablunk::Math
{
	bool decompose_transform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_scale, glm::vec3& out_rotation);
	
}

#endif
