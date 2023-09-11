#pragma once
#ifndef KABLUNK_ML_NETWORK_ILAYER_HPP
#define KABLUNK_ML_NETWORK_ILAYER_HPP

#include "Kablunk/ML/Tensor/Tensor.hpp"
#include "Kablunk/ML/Network/ActivationFunction.hpp"

#include "Kablunk/Core/CoreTypes.h"

namespace kb::ml::network
{
	template <typename T = f32, typename tensor_t = kb::ml::tensor::Tensor<T, 2ull>>
	class ILayer
	{
	public:
		using value_t = T;
		using layer_tensor_t = tensor_t;
	public:
		virtual ~ILayer() = default;

		// \brief return the number of nodes (potentially including bias node)
		virtual size_t get_node_count() const = 0;

		// \brief get the number of inputs nodes
		virtual size_t get_input_node_count() const = 0;

		// \brief get the number of output nodes
		virtual size_t get_output_node_count() const = 0;

		// \brief get the layer type
		virtual layer_type_t get_layer_type() const = 0;

		// \brief return whether the layer has a bias node
		virtual bool has_bias_node() const = 0;

		// \brief get the size of the layer
		virtual size_t size() const = 0;

		// \brief feed inputs through layer
		virtual layer_tensor_t forward(const layer_tensor_t& values) const = 0;

		// \brief get underlying weights for the layer
		// \return reference to the 2x2 tensor of weights
		virtual layer_tensor_t& get_weights() = 0;

		// \brief get underlying weights for the layer
		virtual const layer_tensor_t& get_weights() const = 0;

		// \brief set an activation function for this layer
		virtual void set_activation_func(activation_func_t<value_t> new_activation_function) = 0;
	};

}

#endif
