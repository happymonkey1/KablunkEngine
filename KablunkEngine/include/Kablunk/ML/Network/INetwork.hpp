#pragma once
#ifndef KABLUNK_ML_NETWORK_INETWORK_HPP
#define KABLUNK_ML_NETWORK_INETWORK_HPP

#include "Kablunk/ML/Network/v1/LinearLayer.hpp"
#include "Kablunk/ML/Optimizer/Optimizer.hpp"

#include <stdint.h>

namespace Kablunk::ml::network
{

	template <typename T>
	class INetwork
	{
	public:
		using value_t = T;
		using network_tensor_t = Kablunk::ml::tensor::Tensor<value_t, 2>;
		using optimizer_t = Kablunk::ml::optimizer::IOptimizer<value_t>;
	public:
		virtual ~INetwork() = default;
		// fit the data to the network
		virtual void fit(const network_tensor_t& inputs) = 0;
		// feed inputs forward through the network
		virtual network_tensor_t forward(const network_tensor_t& input) = 0;
		// compute gradients via an optimizer and update internal weights
		virtual void step(const network_tensor_t& y_true) = 0;
		// get the number of layers in the network
		virtual size_t get_layer_count() const = 0;
		// get the optimizer for the network
		// returns nullptr if back-propagation (training) of the network is not required.
		virtual optimizer_t* get_optimizer() = 0;
		// get the optimizer for the network
		// returns nullptr if back-propagation (training) of the network is not required.
		virtual const optimizer_t* get_optimizer() const = 0;
		// get a layer by index from the network
		virtual ILayer<value_t, network_tensor_t>* get_layer(size_t layer_index) = 0;
	};

}

#endif
