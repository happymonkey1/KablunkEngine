#ifndef KABLUNK_RENDERER_RENDERER_TYPES_H
#define KABLUNK_RENDERER_RENDERER_TYPES_H

#include <glm/glm.hpp>
#include <cstdint>

namespace Kablunk
{
	using RendererID = uint32_t;

	struct Float3GL
	{
		float X = { 0.0f };
		float Y = { 0.0f };
		float Z = { 0.0f };

		Float3GL(float x, float y, float z) : X{ x }, Y{ y }, Z{ z } {}
		Float3GL(const glm::vec3& v) : X{ v.x }, Y{ v.y }, Z{ v.z } {}
	};
}

#endif
