#include <kablunkpch.h>

#include "Kablunk/ML/Test/test_SimpleNN.h"

#include "Kablunk/ML/Network/v1/SimpleNeuralNetwork.hpp"

using namespace Kablunk::ml::network;

namespace Kablunk::ml::test
{
	// forward declarations
	void test_nn_constructor();

	void test_simple_nn_main()
	{
		test_nn_constructor();
	}

	void test_nn_constructor()
	{
		KB_CORE_TRACE("[Test] simple neural network constructor test");

		size_t input_size = 4;
		size_t hidden_size = 3;
		size_t output_size = 1;
		v1::SimpleNeuralNetwork<> nn{
			{
				new v1::LinearLayer{ layer_type_t::Input, input_size, hidden_size },
				new v1::LinearLayer{ layer_type_t::Hidden, hidden_size, hidden_size },
				new v1::LinearLayer{ layer_type_t::Output, hidden_size, output_size, &ml::network::sigmoid }
			}
		};

		KB_CORE_TRACE("[Test] Network layer count = {}", nn.get_layer_count());
		KB_CORE_TRACE("[Test] Network with weights = 0");

		Kablunk::ml::tensor::Tensor<f32, 2> tensor{ 1, input_size };
		tensor.fill(1.0f);
		auto output = nn.forward(tensor);

		KB_CORE_TRACE("nn tensor output: {}", output.to_string());

		f32 actual_output = output[0];
		f32 expected_output = 0.5f;
		if (output.get_size() == output_size && actual_output == expected_output)
			KB_CORE_INFO("[Test] nn with weights = 0 passed!");
		else
		{
			KB_CORE_ERROR("[Test] nn with weights = 0 failed");
			KB_CORE_ASSERT(false, "???");
		}

		KB_CORE_TRACE("[Test] nn with weights = 1.0f");

		nn.get_layer(0)->get_weights().fill(1.0f);
		nn.get_layer(1)->get_weights().fill(1.0f);
		nn.get_layer(2)->get_weights().fill(1.0f);

		output = nn.forward(tensor);
		actual_output = output[0];
		expected_output = ml::network::sigmoid(36.0f);
		if (output.get_size() == output_size && expected_output == actual_output)
			KB_CORE_INFO("[Test] nn {} == {} passed!", expected_output, actual_output);
		else
		{
			KB_CORE_ERROR("[Test] nn {} != {} failed!", expected_output, actual_output);
			KB_CORE_ASSERT(false, "???");
		}

		KB_CORE_TRACE("[Test] done");

		KB_CORE_TRACE("[Test] nn destructor");

		size_t nn_count = 100000;
		{
			std::vector<v1::SimpleNeuralNetwork<>> nns;
			nns.reserve(nn_count);
			for (size_t i = 0; i < nn_count; ++i)
			{
				input_size = 10;
				hidden_size = 5;
				output_size = 3;
				nns.push_back(v1::SimpleNeuralNetwork<>(
					{
						new v1::LinearLayer{ layer_type_t::Input, input_size, hidden_size },
						new v1::LinearLayer{ layer_type_t::Hidden, hidden_size, hidden_size },
						new v1::LinearLayer{ layer_type_t::Output, hidden_size, output_size, &ml::network::sigmoid }
					},
					new ml::optimizer::B4{ 0.002f, new ml::optimizer::negative_log_loss{} }
				));
			}

			KB_CORE_TRACE("[Test] nn destructor test just created {} NNs!", nn_count);
			nns.clear();
			KB_CORE_TRACE("[Test] nn destructor test just freed {} NNs!", nn_count);
		}


		KB_CORE_TRACE("[Test] nn destructor done!");

	}

}
