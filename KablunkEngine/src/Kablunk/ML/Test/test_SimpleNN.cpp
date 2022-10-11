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
		size_t input_size = 4;
		size_t hidden_size = 3;
		size_t output_size = 1;
		v1::SimpleNeuralNetwork<> nn{
			{
				new v1::LinearLayer<>{ layer_type_t::Input, input_size, hidden_size },
				new v1::LinearLayer<>{ layer_type_t::Hidden, hidden_size, hidden_size },
				new v1::LinearLayer<>{ layer_type_t::Output, hidden_size, output_size }
			}
		};

		KB_CORE_TRACE("[Test] simple neural network constructor test");

		KB_CORE_TRACE("[Test] Network layer count = {}", nn.get_layer_count());

		KB_CORE_TRACE("[Test] done");

	}

}
