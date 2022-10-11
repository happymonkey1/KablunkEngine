#pragma once
#ifndef KABLUNK_ML_OPTIMIZER_B4_H
#define KABLUNK_ML_OPTIMIZER_B4_H

#include "Kablunk/Core/CoreTypes.h"

#include "Kablunk/ML/Optimizer/Optimizer.hpp"
#include "Kablunk/ML/Optimizer/LossFunction.hpp"
#include "Kablunk/ML/Tensor/Tensor.hpp"

namespace Kablunk::ml::optimizer
{
	// the b4 optimizer
	template <typename T = f32>
	class B4 : public IOptimizer<T>
	{
	public:
		using value_t = T;
		using loss_function_t = ILossFunction<T>;
		using tensor_t = IOptimizer<T>::tensor_t;
	public:
		B4() = delete;
		explicit B4(f32 learning_rate, loss_function_t* loss_function = new negative_log_loss{})
			: m_learning_rate{ learning_rate }, m_loss_function{ loss_function }
		{

		}

		virtual ~B4() override
		{
			delete m_loss_function;
		}
		
		virtual tensor_t loss(const tensor_t& values) override
		{
			KB_CORE_ASSERT(m_loss_function, "loss function base class pointer not set!");
			return m_loss_function->loss(values);
		}

		virtual void backward(const tensor_t& values) override
		{
			KB_CORE_ASSERT(m_loss_function, "loss function base class pointer not set!");
			// #TODO replace with adam instead of steepest descent
			*m_gradients = (*m_gradients) - m_loss_function->grad(values).mul(m_learning_rate);
		}

		virtual void zero_grad() override { m_gradients->fill(0.0f); }

		virtual tensor_t& get_gradients(size_t layer_index) override { return *m_gradients; }
		virtual const tensor_t& get_gradients(size_t layer_index) const override { return *m_gradients; }

		virtual void cache_output(size_t layer_index, const tensor_t& layer_output) override 
		{
			// #TODO make sure we reserve first?
			m_cached_layer_outputs[layer_index] = layer_output;
		}

		virtual void reserve_output_cache(size_t new_size) override { m_cached_layer_outputs.reserve(new_size); }
	private:
		// pointer to the gradients
		tensor_t* m_gradients = nullptr;
		// learning rate (alpha) of the optimizer
		f32 m_learning_rate;
		// loss function
		loss_function_t* m_loss_function;
		// cached outputs for layers
		// #TODO better data structure?
		std::vector<tensor_t> m_cached_layer_outputs;
	};
}

#endif
