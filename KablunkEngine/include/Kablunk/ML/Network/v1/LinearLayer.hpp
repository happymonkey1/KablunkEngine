#pragma once
#ifndef KABLUNK_ML_NETWORK_V1_LAYER_H
#define KABLUNK_ML_NETWORK_V1_LAYER_H

#include "Kablunk/ML/Network/ActivationFunction.hpp"
#include "Kablunk/ML/Network/LayerType.h"
#include "Kablunk/ML/Network/v1/Node.hpp"
#include "Kablunk/ML/Tensor/Tensor.hpp"

#include "Kablunk/ML/Network/ILayer.hpp"

#include <vector>
#include <stdint.h>


namespace Kablunk::ml::network::v1
{
	// #TODO stack allocated layer?
	// computes activation(x.dot(w) + b)
	template <typename T = f32, typename tensor_t = Kablunk::ml::tensor::Tensor<T, 2ull>>
	class LinearLayer : public ILayer<T, tensor_t>
	{
	public:
		LinearLayer() = delete;

		// #TODO figure out if it is possible to keep m_weights uninitialized so we don't spend time constructing it before immediately being replaced
		
		/*
		 * \brief linear layer constructor that takes input and output size, an activation function, and bias node
		 * \param layer_type enum for the layer type (input, hidden, or output)
		 * \param input_dimension size of the input for this layer
		 * \param output_dimension size of the output for this layer
		 * \param activation_func activation function applied to the output of the layer (pass nullptr for a linear combination of the output)
		 * \param use_bias_node flag for whether the layer has a bias node, only valid on the input layer
		 */
		explicit LinearLayer(
			layer_type_t				layer_type,
			size_t						input_dimension,
			size_t						output_dimension,
			activation_func_t<value_t>	activation_func = &relu<value_t>,
			bool						use_bias_node = false
		)
			: m_activation_func{ activation_func }, m_use_bias_node{ use_bias_node }
		{
			// assert bias node only in input layer
			if (m_use_bias_node)
				KB_CORE_ASSERT(m_layer_type == layer_type_t::Input, "bias node can only be in input layer!");

			KB_CORE_ASSERT(layer_type != layer_type_t::NONE, "layer type cannot be none!");
			KB_CORE_ASSERT(input_dimension >= 1, "no inputs?");
			KB_CORE_ASSERT(output_dimension >= 1, "no outputs?");

			// create weight tensor
			size_t actual_node_count = output_dimension + (m_use_bias_node ? 1ull : 0ull);
			m_weights = layer_tensor_t{ input_dimension, actual_node_count };
			m_weights.fill(0.0f);

			//KB_CORE_TRACE("[ml::network::LinearLayer] Created layer {}x{}", input_dimension, actual_node_count);
		}

		virtual ~LinearLayer() override = default;

		// \brief return the number of nodes (potentially including bias node)
		virtual size_t get_node_count() const override { return m_weights.get_dimension(1); }

		// \brief get the number of inputs nodes
		virtual size_t get_input_node_count() const override { return m_weights.get_dimension(0); }

		// \brief get the number of output nodes
		virtual size_t get_output_node_count() const override { return m_weights.get_dimension(1); }

		// \brief get the layer type
		virtual layer_type_t get_layer_type() const override { return m_layer_type; }

		// \brief return whether the layer has a bias node
		virtual bool has_bias_node() const override { return m_use_bias_node; }

		// \brief get the size of the layer
		virtual size_t size() const override { return m_weights.get_dimension(1); }

		/*
		 * \brief feed inputs through layer
		 * \param 2x2 tensor of input data
		 * \return 2x2 tensor where output = input dot weights
		 */
		virtual layer_tensor_t forward(const layer_tensor_t& values) const override
		{
			// compute input.dot(weights)
			layer_tensor_t output = values.dot(m_weights);

			// apply activation function if there is one
			if (m_activation_func)
				for (value_t& val : output)
					val = m_activation_func(val);

			return output;
		}

		// \brief get underlying weights for the layer
		virtual layer_tensor_t& get_weights() override { return m_weights; }

		// \brief get underlying weights for the layer
		virtual const layer_tensor_t& get_weights() const override { return m_weights; }

		// \brief set an activation function for this layer
		virtual void set_activation_func(activation_func_t<value_t> new_activation_function) override
		{
			if (!new_activation_function)
				KB_CORE_WARN("[ml::network::LinearLayer] Potential error, setting activation function for layer to null...");

			m_activation_func = new_activation_function;
		}
	private:
		// \brief activation function for each of the nodes in the layer
		activation_func_t<value_t> m_activation_func = nullptr;
		// \brief layer type enum
		layer_type_t m_layer_type = layer_type_t::NONE;
		// \brief flag for whether there is a bias node (only applicable for input layer)
		bool m_use_bias_node = false;
		// \brief weights matrix
		layer_tensor_t m_weights;
	};

}

#endif
