#pragma once
#ifndef KABLUNK_ML_OPTIMIZER_LOSS_FUNCTION_H
#define KABLUNK_ML_OPTIMIZER_LOSS_FUNCTION_H

#include "Kablunk/ML/Tensor/Tensor.hpp"
#include "Kablunk/ML/Network/ActivationFunction.hpp"

#include "Kablunk/Core/CoreTypes.h"

#include <glm/glm.hpp>

namespace Kablunk::ml::optimizer
{

	template <typename T = f32>
	class ILossFunction
	{
	public:
		using value_t = T;
		using tensor_t = Kablunk::ml::tensor::Tensor<T, 2>;
	public:
		virtual ~ILossFunction() = default;

		// compute the loss 
		virtual tensor_t loss(const tensor_t& y_true, const tensor_t& y_pred) const = 0;

		// compute the gradient of the loss
		virtual tensor_t grad(const tensor_t& y_true, const tensor_t& y_pred) const = 0;
	};
	

	template <typename T = f32>
	class negative_log_loss : public ILossFunction<T>
	{
	public:
		using value_t = T;
		using tensor_t = Kablunk::ml::tensor::Tensor<T, 2>;
	public:
		virtual ~negative_log_loss() = default;

		// compute the loss 
		virtual tensor_t loss(const tensor_t& y_true, const tensor_t& y_pred) const override
		{
			// nll = -(1/n) y log( sig( y_pred ) ) + (1 - y) * log( -sig( y_pred ) )
			// #TODO cache lambda instead of re-creating each func call
			/*auto loss_f = [](value_t v) -> value_t {
				return - y_true * glm::log(static_cast<value_t>(v)); 
			};*/
			tensor_t output{ y_pred.get_dimensions() };

			// #TODO use vector math

			//output = (-1 * y_true) * glm::log(y_pred) - (1 - y_true) * glm::log((-1 * y_pred));

			return output; /// static_cast<value_t>(output.get_dimension(1));
		}

		// compute the gradient of the loss
		virtual tensor_t grad(const tensor_t& y_true, const tensor_t& y_pred) const
		{
			// #TODO cache lambda instead of re-creating each func call
			auto d_sigmoid = [](value_t v) -> value_t { return Kablunk::ml::network::sigmoid(v) * (1 - Kablunk::ml::network::sigmoid(v)); };
			tensor_t output{ y_pred };

			return output.apply(d_sigmoid);
		}
	};

}

#endif
