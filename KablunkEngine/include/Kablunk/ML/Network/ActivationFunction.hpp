#pragma once
#ifndef KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H
#define KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H

#include "Kablunk/ML/Tensor/Tensor.hpp"
#include <glm/glm.hpp>

namespace Kablunk::ml::network
{
	// #TODO add constraints after transition to c++20 
	template <typename T = f32>
	using activation_func_t = T(*)(T);

	template <typename T = f32>
	inline T relu(T value) { return glm::max(static_cast<T>(0.0f), value); }

	template <typename T = f32>
	inline T sigmoid(T value) { return glm::exp(value) / (glm::exp(value) + 1); }
}

#endif
