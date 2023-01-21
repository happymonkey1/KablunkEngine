#pragma once
#ifndef KABLUNK_ML_NETWORK_V1_NODE_H
#define KABLUNK_ML_NETWORK_V1_NODE_H

#include "Kablunk/ML/Network/ActivationFunction.hpp"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Utilities/Random/FastRandom.h"

#include <vector>
#include <stdint.h>

namespace Kablunk::ml::network::v1
{
	using node_id_t = uuid::uuid64;

	template <typename T, typename weights_array_t = std::vector<T>>
	class Node
	{
	public:
		using value_t = T;
		using input_array_t = weights_array_t;
	public:
		Node() = delete;
		Node(size_t dimension, weights_array_t&& initial_weights = {})
			: m_id{ uuid::generate() }
		{
			KB_CORE_ASSERT(dimension > 0, "dimension of weights must be > 0!");


			if (initial_weights.size() > 0)
			{
				m_weights = std::move(initial_weights);
				m_initialized = true;
			}

			// uniform random initialization in range=[-1,1]
			if (!m_initialized)
			{
				m_weights.reserve(dimension);

				// #TODO figure out better type casting
				for (size_t i = 0; i < dimension; ++i)
					m_weights[i] = static_cast<value_t>(-1.0f + util::random::random_f32() + (1.0f + 1.0f));

				m_initialized = true;
			}
		}

		~Node() = default;

		// reserve heap space for input dimension
		void reserve(size_t input_dim)
		{
			m_weights.reserve(input_dim);
		}

		// computes activation_func(input.dot(weights))
		value_t activate(const input_array_t& inputs, activation_func_t<value_t> activation_func) const
		{
			KB_CORE_ASSERT(inputs.size() == m_weights.size(), "input dimension does not match weights dimension!");
			KB_CORE_ASSERT(m_weights.size() > 0, "weights size == 0?");

			T accumulation{};
			for (size_t i = 0; i < inputs.size(); ++i)
				accumulation += inputs[i] * m_weights[i];
			
			return activation_func(accumulation);
		}

		node_id_t get_node_id() const { return m_id; }

		const weights_array_t& get_weights() const { return m_weights; }
	private:
		// weights list
		weights_array_t m_weights;
		// whether the node has been initialized (weights)
		bool m_initialized = false;
		// unique identifier for node
		node_id_t m_id = uuid::nil_uuid;
	};

	template <typename T = f32, typename weights_array_t = std::vector<T>>
	using node_t = Node<T, weights_array_t>;
}

#endif
