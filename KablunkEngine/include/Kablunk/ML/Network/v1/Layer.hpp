#pragma once
#ifndef KABLUNK_ML_NETWORK_V1_LAYER_H
#define KABLUNK_ML_NETWORK_V1_LAYER_H

#include "Kablunk/ML/Network/ActivationFunction.hpp"
#include "Kablunk/ML/Network/LayerType.h"
#include "Kablunk/ML/Network/v1/Node.hpp"
#include "Kablunk/ML/Tensor/Tensor.hpp"

#include <vector>
#include <stdint.h>


namespace Kablunk::ml::network::v1
{

	// #TODO add concept for requiring weights_array_t to have a "reserve" function
	// #TODO add concept for requiring weights_array_t to have iterators
	// #TODO stack allocated layer?
	template <typename T = f32, typename nd_array = tensor_1d_t<T>>
	class Layer
	{
	public:
		using value_t = T;
		using nd_array_t = nd_array;
		using neuron_t = node_t<value_t, nd_array_t>;
		using nodes_array_t = std::vector<neuron_t>;
	public:
		Layer() = delete;
		Layer(layer_type_t layer_type, size_t input_dimension, size_t nodes_count, activation_func_t<value_t> activation_func = &relu<value_t>, bool use_bias_node = false)
			: m_activation_func{ activation_func }, m_use_bias_node{ use_bias_node }
		{
			// assert bias node only in input layer
			static_assert((layer_type == layer_type_t::Input) ^ use_bias_node, "bias node can only be in input layer!");
			
			KB_CORE_ASSERT(layer_type != layer_type_t::NONE, "layer type cannot be none!");

			// reserve space for nodes
			size_t node_count = nodes_count + m_use_bias_node ? 1ull : 0ull;
			m_nodes.reserve(node_count);

			for (size_t i = 0; i < nodes_count; ++i)
				m_nodes[i] = neuron_t{ input_dimension };
		}

		~Layer() = default;

		// return the number of nodes (potentially including bias node)
		size_t get_node_count() const { return m_nodes.size(); }

		// get the layer type
		layer_type_t get_layer_type() const { return m_layer_type; }

		// return whether the layer has a bias node
		bool has_bias_node() const { return m_use_bias_node; }

		// get the size of the layer
		size_t size() const { return m_nodes.size(); }

		// feed inputs through layer
		nd_array_t feed_forward(const nd_array_t& values) const
		{
			nd_array_t output{ values.size() };
			
			// activate each neuron and store
			for (size_t i = 0; i < size(); ++i)
				output[i] = m_nodes[i].activate(values, m_activation_func);

			return output;
		}
	private:
		// activation function for each of the nodes in the layer
		activation_func_t m_activation_func = nullptr;
		// layer type enum
		layer_type_t m_layer_type = layer_type_t::NONE;
		// flag for whether there is a bias node (only applicable for input layer)
		bool m_use_bias_node = false;
		// nodes list for this layer
		nodes_array_t m_nodes;
	};

}

#endif
