#pragma once
#ifndef KABLUNK_ML_NETWORK_ILAYER_HPP
#define KABLUNK_ML_NETWORK_ILAYER_HPP

#include "Kablunk/ML/Tensor/Tensor.hpp"

#include "Kablunk/Core/CoreTypes.h"

namespace Kablunk::ml::network
{
	template <typename T = f32, typename tensor_t = Kablunk::ml::tensor::Tensor<T, 2ull>>
	class ILayer
	{
	public:
		using value_t = T;
		using layer_tensor_t = tensor_t;
	public:
		virtual ~ILayer() = default;

		// return the number of nodes (potentially including bias node)
		virtual size_t get_node_count() const = 0;

		// get the layer type
		virtual layer_type_t get_layer_type() const = 0;

		// return whether the layer has a bias node
		virtual bool has_bias_node() const = 0;

		// get the size of the layer
		virtual size_t size() const = 0;

		// feed inputs through layer
		virtual layer_tensor_t forward(const layer_tensor_t& values) const = 0;

		// get underlying weights for the layer
		virtual layer_tensor_t& get_weights() = 0;
		virtual const layer_tensor_t& get_weights() const = 0;
	};

}

#endif
