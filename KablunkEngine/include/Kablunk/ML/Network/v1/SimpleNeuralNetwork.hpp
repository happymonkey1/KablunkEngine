#pragma once
#ifndef KABLUNK_ML_NETWORK_V1_SIMPLE_NEURAL_NETWORK_HPP
#define KABLUNK_ML_NETWORK_V1_SIMPLE_NEURAL_NETWORK_HPP

#include "Kablunk/ML/Network/INetwork.hpp"
#include "Kablunk/ML/Network/ILayer.hpp"

#include "Kablunk/ML/Optimizer/B4.hpp"

#include <array>

namespace Kablunk::ml::network::v1
{
	// #TODO layers with different underlying types
	// for example int16 layer -> int8 layer -> int8 layer, all in one network


	template <typename T = f32>
	class SimpleNeuralNetwork : public INetwork<T>
	{
	public:
		using value_t = T;
		using layer_t = Kablunk::ml::network::ILayer<T>;
		using optimizer_t = Kablunk::ml::network::INetwork<T>::optimizer_t;
		// #TODO this may not be correct
		using network_tensor_t = Kablunk::ml::tensor::Tensor<value_t, 2>;
	public:
		SimpleNeuralNetwork(
			const std::initializer_list<layer_t*>& layers, 
			optimizer_t* optimizer = new Kablunk::ml::optimizer::B4<value_t>{ 0.002f, new Kablunk::ml::optimizer::negative_log_loss<T>{} }
		)
			: m_layers{ layers }, m_optimizer{ optimizer }
		{
			KB_CORE_ASSERT(layers.size() >= 1, "no layers?");

			// reserve space for optimizer output cache
			if (optimizer)
				m_optimizer->reserve_output_cache(m_layers.size());
		}

		// move constructor
		SimpleNeuralNetwork(
			SimpleNeuralNetwork<>&& other
		) noexcept
			: m_layers{ std::move(other.m_layers) }, m_optimizer{ other.m_optimizer }, m_data{ std::move(other.m_data) }
		{
			other.m_layers.clear();
			other.m_optimizer = nullptr;
		}

		virtual ~SimpleNeuralNetwork() override
		{
			// #TODO figure out how to stack allocate optimizers so they are not on the heap...
			if (m_optimizer)
				delete m_optimizer;

			// #TODO figure out how to stack allocate layers so they are not on the heap...
			for (layer_t* layer : m_layers)
				delete layer;
		}

		// get the amount of layers in the network
		virtual size_t get_layer_count() const override { return m_layers.size(); }

		// fit the data to the network
		virtual void fit(const network_tensor_t& inputs) override
		{
			KB_CORE_ASSERT(false, "not implemented");
		}

		// feed inputs forward through the network
		virtual network_tensor_t forward(const network_tensor_t& input) override
		{ 
			network_tensor_t output{ input };
			size_t layer_index = 0;
			for (layer_t* layer : m_layers)
			{
				output = layer->forward(output);
				if (m_optimizer)
					m_optimizer->cache_output(layer_index, output);
			}

			return std::move(output);
		}

		// compute gradients via an optimizer and update internal weights
		virtual void step(const network_tensor_t& y_true) override
		{
			KB_CORE_ASSERT(m_optimizer, "no optimizer set!");
			m_optimizer->backward(m_data);
		}

		// get the optimizer for the network
		// returns nullptr if back-propagation (training) of the network is not required.
		virtual INetwork<T>::optimizer_t* get_optimizer() { return m_optimizer; }

		// get the optimizer for the network
		// returns nullptr if back-propagation (training) of the network is not required.
		virtual const INetwork<T>::optimizer_t* get_optimizer() const { return m_optimizer; }

		// get a layer by index from the network
		virtual ILayer<value_t, network_tensor_t>* get_layer(size_t layer_index) { return m_layers[layer_index]; } // #TODO buffer overflow?

		// get a layer by index from the network
		virtual const ILayer<value_t, network_tensor_t>* get_layer(size_t layer_index) const { return m_layers[layer_index]; } // #TODO buffer overflow?
	private:
		network_tensor_t m_data;
		// stack allocated array for layers of the network
		std::vector<layer_t*> m_layers;
		// optimizer for the network
		INetwork<T>::optimizer_t* m_optimizer = nullptr;
	};

}

#endif
