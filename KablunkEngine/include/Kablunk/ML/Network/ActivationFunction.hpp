#pragma once
#ifndef KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H
#define KABLUNK_ML_NETWORK_ACTIVATION_FUNCTION_H

#include <glm/glm.hpp>

namespace Kablunk::ml::network
{

	// #TODO add constraints after transition to c++20 
	template <typename T>
	using activation_func_t = T(*)(T);

	template <typename T>
	inline T relu(T value) { return glm::max(0, value); }

	template <typename T>
	inline T sigmoid(T value) { return glm::exp(value) / (glm::exp(value) + 1); }
}

#endif
