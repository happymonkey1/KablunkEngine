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

		// \brief fit the data to the network
		virtual void fit(const network_tensor_t& inputs) = 0;

		// \brief feed inputs forward through the network
		virtual network_tensor_t forward(const network_tensor_t& input) = 0;

		// \brief compute gradients via an optimizer and update internal weights
		virtual void step(const network_tensor_t& y_true) = 0;

		// \brief get the number of layers in the network
		virtual size_t get_layer_count() const = 0;

		// \brief get the optimizer for the network
		// \return pointer to the base class of the optimizer, or nullptr if back-propagation (training) of the network is not required.
		virtual optimizer_t* get_optimizer() = 0;

		// \brief get the optimizer for the network
		// \return pointer to the base class of the optimizer, or nullptr if back-propagation (training) of the network is not required.
		virtual const optimizer_t* get_optimizer() const = 0;

		// \brief get a layer by index from the network
		// \param layer_index index of the layer
		// \return pointer to the base class of the layer at the given index
		virtual ILayer<value_t, network_tensor_t>* get_layer(size_t layer_index) = 0;

		// \brief get a layer by index from the network
		// \param layer_index index of the layer
		// \return pointer to the base class of the layer at the given index
		virtual const ILayer<value_t, network_tensor_t>* get_layer(size_t layer_index) const = 0;
	};

}

#endif
