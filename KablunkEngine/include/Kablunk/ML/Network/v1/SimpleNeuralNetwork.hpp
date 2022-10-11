#pragma once
#ifndef KABLUNK_ML_NETWORK_V1_SIMPLE_NEURAL_NETWORK_HPP
#define KABLUNK_ML_NETWORK_V1_SIMPLE_NEURAL_NETWORK_HPP

#include "Kablunk/ML/Network/INetwork.hpp"
#include "Kablunk/ML/Network/ILayer.hpp"

#include "Kablunk/ML/Optimizer/B4.hpp"

#include <array>

namespace Kablunk::ml::network::v1
{
	template <typename T = f32>
	class SimpleNeuralNetwork : public INetwork<T>
	{
	public:
		using value_t = T;
		// #TODO replace with ILayer*
		using layer_t = Kablunk::ml::network::ILayer<T>;
		using optimizer_t = Kablunk::ml::network::INetwork<T>::optimizer_t;
	public:
		SimpleNeuralNetwork(const std::initializer_list<layer_t*>& layers, 
			optimizer_t* optimizer = new Kablunk::ml::optimizer::B4<value_t>{ 0.002f, new Kablunk::ml::optimizer::negative_log_loss<T>{} })
			: m_layers{ layers }, m_optimizer{ optimizer }
		{

			// #TODO assert that passed in layer args are of base type ILayer
			KB_CORE_ASSERT(layers.size() >= 1, "no layers?");

			// reserve space for optimizer output cache
			if (optimizer)
				m_optimizer->reserve_output_cache(layers.size());
		}

		virtual ~SimpleNeuralNetwork() override
		{
			delete m_optimizer;
		}

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
		virtual INetwork::optimizer_t& get_optimizer() { return *m_optimizer; };
		// get the optimizer for the network
		virtual const INetwork::optimizer_t& get_optimizer() const { return *m_optimizer; };

	private:
		network_tensor_t m_data;
		// stack allocated array for layers of the network
		std::vector<layer_t*> m_layers;
		// optimizer for the network
		INetwork::optimizer_t* m_optimizer;
	};

}

#endif
