#include "kablunkpch.h"
#include "Kablunk/ML/Test/test_Tensor.h"

#include "Kablunk/ML/Tensor/Tensor.hpp"

namespace kb::ml::tensor
{
	// #NOTE this whole file is temporary and badly coded. 
	// #TODO refine code

	void test_tensor_constructors();
	void test_tensor_allocations();
	void test_tensor_dot();
	void test_tensor_operations();

	void test_tensor_main()
	{

		test_tensor_constructors();
		test_tensor_allocations();
		test_tensor_dot();
		test_tensor_operations();

	}

	void test_tensor_constructors()
	{
		KB_CORE_INFO("\n[Test] [Tensor] Starting constructor test");
		int total_tests = 0;
		int tests_passed = 0;

		Tensor<f32, 1> tensor_a{ 5 };

		KB_CORE_TRACE("tensor_a:\n  {}", tensor_a.to_string());
		total_tests++;
		if (tensor_a.get_size() == 5)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		KB_CORE_TRACE("[Test] [Tensor] Starting fill test");

		tensor_a.fill(1.0f);
		KB_CORE_TRACE("tensor_a:\n  {}", tensor_a.to_string());
		total_tests++;
		bool passed = true;
		for (size_t i = 0; i < tensor_a.get_size(); ++i)
			if (*tensor_a.get(i) != 1.0f)
				passed = false;

		if (passed)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		KB_CORE_TRACE("[Test] destructor test");

		{
			size_t tensor_count = 1000;
			size_t elements = 100000;
			std::vector<Tensor<u64, 1>> tensors;
			tensors.reserve(tensor_count);
			for (size_t i = 0; i < tensor_count; ++i)
				tensors.emplace_back(Tensor<u64, 1>{ elements });

			tensors.clear();
		}
		
		if (tests_passed == total_tests)
			KB_CORE_INFO("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		else
			KB_CORE_ERROR("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		KB_CORE_TRACE("[Test] [Tensor] Finished constructor test");
	}

	void test_tensor_allocations()
	{
		KB_CORE_TRACE("[Test] [Tensor] Starting allocation test");
		int total_tests = 0;
		int tests_passed = 0;


		if (tests_passed == total_tests)
			KB_CORE_INFO("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		else
			KB_CORE_ERROR("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		KB_CORE_TRACE("[Test] [Tensor] Finished allocation test");
	}

	void test_tensor_dot()
	{
		KB_CORE_TRACE("[Test] [Tensor] Starting dot product test");
		int total_tests = 0;
		int tests_passed = 0;

		Tensor<f32, 2> tensor_a{ 2, 2 };
		tensor_a.fill(1.0f);

		Tensor<f32, 2> tensor_b{ 2, 2 };
		tensor_b.fill(1.0f);

		Tensor<f32, 2> tensor_c = tensor_a.dot(tensor_b);

		KB_CORE_TRACE("tensor_a:\n  {}", tensor_a.to_string());
		KB_CORE_TRACE("tensor_b:\n  {}", tensor_b.to_string());
		KB_CORE_TRACE("tensor_c:\n  {}", tensor_c.to_string());
		total_tests++;
		bool passed = true;
		for (size_t i = 0; i < tensor_c.get_size(); ++i)
			if (*tensor_c.get(i) != 2.0f)
				passed = false;

		if (passed)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		KB_CORE_TRACE("[Test] test 2x3.dot(3x2)");
		Tensor<f32, 2> tensor_1{ 2, 3 };
		tensor_1.fill(1.0f);
		tensor_1[0] = 1.0f;
		tensor_1[1] = 2.0f;
		tensor_1[2] = 0.0f;
		tensor_1[3] = 2.0f;
		tensor_1[4] = 0.0f;

		Tensor<f32, 2> tensor_2{ 3, 2 };
		tensor_2.fill(1.0f);
		tensor_2[0] = 2.0f;
		tensor_2[4] = 0.0f;

		auto tensor_3 = tensor_1.dot(tensor_2);
		KB_CORE_INFO("tensor_1: {}", tensor_1.to_string());
		KB_CORE_INFO("tensor_2: {}", tensor_2.to_string());
		KB_CORE_INFO("tensor_3: {}", tensor_3.to_string());


		if (tests_passed == total_tests)
			KB_CORE_INFO("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		else
			KB_CORE_ERROR("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		KB_CORE_TRACE("[Test] [Tensor] Finished dot product test");
	}

	void test_tensor_operations()
	{
		KB_CORE_TRACE("[Test] [Tensor] Starting operations test");
		int total_tests = 0;
		int tests_passed = 0;

		Tensor<f32, 2> tensor_a{ 2, 2 };
		tensor_a.fill(1.0f);

		tensor_a[3] = 5.0f;

		KB_CORE_TRACE("tensor_a:\n  {}", tensor_a.to_string());
		total_tests++;
		bool passed = tensor_a[3] == 5.0f;

		if (passed)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		tensor_a.fill(5.0f);

		Tensor<f32, 2> tensor_b{ 2, 2 };
		tensor_b.fill(5.0f);

		Tensor<f32, 2> tensor_c{ 2, 2 };
		tensor_c.fill(2.5f);

		total_tests++;
		passed = (tensor_a == tensor_b) && (tensor_a != tensor_c) && (tensor_b != tensor_c);

		if (passed)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		KB_CORE_TRACE("[Test] tensor * scalar");
		Tensor<f32, 2> tensor_d{ 2, 2 };
		tensor_d.fill(1.0f);
		tensor_d = tensor_d * 5.0f;

		Tensor<f32, 1> tensor_e{ 10 };
		tensor_e.fill(1.0f);
		tensor_e = tensor_e * 2.0f;

		KB_CORE_INFO("tensor_d: {}", tensor_d.to_string());
		KB_CORE_INFO("tensor_e: {}", tensor_e.to_string());

		total_tests++;
		passed = true;
		for (size_t i = 0; i < tensor_d.get_size(); ++i)
			if (tensor_d[i] != 5.0f)
				passed = false;

		for (size_t i = 0; i < tensor_e.get_size(); ++i)
			if (tensor_e[i] != 2.0f)
				passed = false;

		if (passed)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		KB_CORE_TRACE("[Test] tensor iterators");

		Tensor<f32, 1> tensor_f{ 10 };
		tensor_f.fill(69.0f);

		KB_CORE_INFO("tensor_f: {}", tensor_f.to_string());

		total_tests++;
		size_t expected_size = 10;
		size_t actual_size = 0;
		for (f32 val : tensor_f)
		{
			KB_CORE_TRACE("  tensor_e[{}] = {}", actual_size, val);
			actual_size++;
		}

		if (actual_size == expected_size)
		{
			KB_CORE_INFO("[Test #{}] Passed!", total_tests);
			tests_passed++;
		}
		else
		{
			KB_CORE_ERROR("[Test #{}] Failed!", total_tests);
		}

		if (tests_passed == total_tests)
			KB_CORE_INFO("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		else
			KB_CORE_ERROR("[Test]   {}/{} tests passed!", tests_passed, total_tests);
		KB_CORE_TRACE("[Test] [Tensor] Finished operations test\n");
	}

}
