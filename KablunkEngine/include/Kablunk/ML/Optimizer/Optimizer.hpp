#pragma once
#ifndef KABLUNK_ML_OPTIMIZER_OPTIMIZER_H
#define KABLUNK_ML_OPTIMIZER_OPTIMIZER_H

#include "Kablunk/ML/Optimizer/LossFunction.hpp"

#include "Kablunk/Core/CoreTypes.h"

namespace kb::ml::optimizer
{

	// abstract base class for an optimizer
	template <typename T = f32>
	class IOptimizer
	{
	public:
		using value_t = T;
		using tensor_t = kb::ml::tensor::Tensor<value_t, 2ull>;
	public:
		virtual ~IOptimizer() = default;

		virtual tensor_t loss(const tensor_t& values) = 0;
		virtual void backward(const tensor_t& values) = 0;
		virtual void zero_grad() = 0;

		// get the gradients cache for a layer from the optimizer
		virtual tensor_t& get_gradients(size_t layer_index) = 0;
		virtual const tensor_t& get_gradients(size_t layer_index) const = 0;

		// cache output for the layer
		virtual void cache_output(size_t layer_index, const tensor_t& layer_output) = 0;
		// reserve space for the output cache
		virtual void reserve_output_cache(size_t new_size) = 0;
	};

}

#endif
