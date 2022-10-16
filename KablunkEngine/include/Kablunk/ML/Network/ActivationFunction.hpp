#pragma once
#ifndef KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H
#define KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H

#include "Kablunk/ML/Tensor/Tensor.hpp"
#include <glm/glm.hpp>

namespace Kablunk::ml::network
{
	// #TODO add constraints after transition to c++20 

	// \brief type definition for an activation function
	template <typename T = f32>
	using activation_func_t = T(*)(T);

	template <typename T = f32>
	inline T relu(T value) { return glm::max(static_cast<T>(0.0f), value); }

	template <typename T = f32>
	inline T clipped_relu(T value) { return glm::min(glm::max(static_cast<T>(0.0f, value), static_cast<T>(1.0f))); }

	template <typename T = f32>
	inline T sigmoid(T value) { return glm::exp(value) / (glm::exp(value) + static_cast<T>(1.0f)); }

	// #TODO figure out softmax
}

#endif
