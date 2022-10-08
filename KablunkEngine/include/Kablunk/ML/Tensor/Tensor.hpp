#pragma once
#ifndef KABLUNK_ML_TENSOR_TENSOR_H
#define KABLUNK_ML_TENSOR_TENSOR_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/ML/Tensor/TensorBuffer.hpp"

#include <glm/glm.hpp>

#include <array>
#include <string>

namespace Kablunk::ml::tensor
{

	template <typename T, size_t rank>
	class Tensor
	{
	public:
		using value_t = T;
		using indices_t = std::array<size_t, rank>;

		// iterator class
		template <typename T>
		class tensor_iterator_t
		{
		public:
			using iterator_value_t = T;
		public:
			explicit tensor_iterator_t(iterator_value_t* ptr) : m_ptr{ ptr } {}

			tensor_iterator_t operator++() { m_ptr++; return *this; }

			iterator_value_t& operator*() { return *m_ptr; }
			const iterator_value_t& operator*() const { return *m_ptr; }

			bool operator==(const tensor_iterator_t& other) const { return m_ptr == other.m_ptr; }
			bool operator!=(const tensor_iterator_t& other) const { return !(*this == other); }
		private:
			iterator_value_t* m_ptr;
		};

	public:
		/*explicit Tensor() noexcept
			: m_buffer{}, m_rank{ rank } 
		{
		
		}*/
		Tensor() = delete;
		
		// variadic template for tensor with multiple dimensions
		// #TODO c++20 concept to assert other_dimensions_t is numeric value
		template <typename... other_dimensions_t>
		explicit Tensor(size_t first_dimension, other_dimensions_t... other_dimensions) noexcept
			: m_buffer{ first_dimension, other_dimensions... }, m_rank{rank}
		{
			// ensure passed in dimensions matches specified rank
			KB_CORE_ASSERT(rank == sizeof...(other_dimensions) + 1, "[Tensor] Passed in dimensions must match specified rank!");
		}

		Tensor(const Tensor& other) 
			: m_buffer{ other.m_buffer }, m_rank{ other.m_rank }
		{

		}

		// #TODO constructors for tensors with other value_t types

		~Tensor() = default;

		// get the rank of the tensor
		size_t get_rank() const noexcept { return m_rank; }

		// get the dimension of a specific rank
		size_t get_dimension(size_t index) const noexcept { return m_buffer.get_dimensions()[index]; }

		// get dimensions
		dimension_t<rank> get_dimensions() const noexcept { return m_buffer.get_dimensions(); }
		
		// get size (number of elements) in the tensor
		size_t get_size() const noexcept { return m_buffer.size(); }

		// get a pointer to the raw block of memory
		value_t* get() noexcept { return m_buffer.get(); }
		const value_t* get() const noexcept { return m_buffer.get(); }

		// get a specific value by index from the buffer. 
		// #TODO overflow protection
		value_t* get(size_t index) noexcept { return m_buffer.get() + index; }

		value_t* as() noexcept { return m_buffer.as(); }

		// compute the dot product between two tensors
		// from https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm
		Tensor dot(const Tensor& other) const
		{
			const TensorBuffer<value_t, rank>& a = m_buffer;
			const TensorBuffer<value_t, rank>& b = other.m_buffer;

			KB_CORE_ASSERT(m_rank <= 2, "dot product only supported on 1d or 2d tensors!");
			KB_CORE_ASSERT(a.get_dimension(1) == b.get_dimension(0), "A.dot(B) A.cols != B.rows!");

			// #TODO optimize for row-major order
			// nxm dot mxp using the mat mult algorithm
			size_t n = a.get_dimension(0);
			size_t m = a.get_dimension(1);
			size_t p = b.get_dimension(1);
			Tensor resultant{ n, p };
			for (size_t i = 0; i < n; ++i)
				for (size_t j = 0; j < p; ++j)
					for (size_t k = 0; k < m; ++k)
						resultant[{i, j}] += a[{i, k}] * b[{k, j}];

			return std::move(resultant);
		}

		// fill tensor with values
		void fill(value_t fill_value)
		{
			for (size_t i = 0; i < m_buffer.size(); ++i)
				*(m_buffer.get() + i) = fill_value;
		}

		std::string to_string() const
		{
			std::string repr = fmt::format("Tensor(m_rank='{}', m_dimension='{}')", m_rank, dimension_to_string(get_dimensions()));
			std::string values = m_buffer.to_string();

			return fmt::format("{}\n{}", repr, values);
		}

		// =========
		// operators
		// =========

		Tensor& operator=(const Tensor& other)
		{
			m_rank = other.m_rank;

			// copy
			m_buffer = other.m_buffer;

			return *this;
		}

		Tensor& operator=(Tensor&& other)
		{
			m_rank = other.m_rank, other.m_rank = 0;

			m_buffer = std::move(other.m_buffer);
			//other.m_buffer.release();

			return *this;
		}

		// dot product when multiplying by other tensor
		Tensor operator*(const Tensor& other) const { return dot(other); }

		template <typename T>
		Tensor operator*(T scalar) const 
		{ 
			static_assert(std::is_arithmetic<T>::value, "[Tensor] type T is not a scalar!");
			Tensor resultant{ *this };

			for (size_t i = 0; i < resultant.get_size(); ++i)
				resultant[i] *= scalar;

			return std::move(resultant);
		}

		template <typename T>
		Tensor operator*=(T scalar)
		{
			static_assert(std::is_arithmetic<T>::value, "[Tensor] type T is not a scalar!");
			for (size_t i = 0; i < get_size(); ++i)
				m_buffer[i] *= scalar;

			return *this;
		}

		value_t& operator[](size_t index) { return m_buffer[index]; }
		const value_t& operator[](size_t index) const { return m_buffer[index]; }

		value_t& operator[](const indices_t& indices) { return m_buffer[indices]; }
		const value_t& operator[](const indices_t& indices) const { return m_buffer[indices]; }

		// equality comparison operator
		bool operator==(const Tensor& other) const
		{
			for (size_t i = 0; i < m_buffer.size(); ++i)
				if (m_buffer[i] != other.m_buffer[i])
					return false;

			return true;
		}

		// iterator operators
		tensor_iterator_t<value_t> begin() { return tensor_iterator_t<value_t>( m_buffer.get() ); }
		tensor_iterator_t<value_t> end() { return tensor_iterator_t<value_t>( m_buffer.get() + m_buffer.size() ); }

		const tensor_iterator_t<value_t> cbegin() const { return tensor_iterator_t<value_t>( m_buffer.get() ); }
		const tensor_iterator_t<value_t> cend() const { return tensor_iterator_t<value_t>( m_buffer.get() + m_buffer.size() ); }

		// inequality comparison operator
		bool operator!=(const Tensor& other) const { return !(*this == other); }
	private:
		// rank (dimensionality) of the tensor
		size_t m_rank;
		// block of memory
		TensorBuffer<value_t, rank> m_buffer;
	};

}

namespace Kablunk::ml
{
	template <typename T = f32>
	using tensor_1d_t = Kablunk::ml::tensor::Tensor<T, 1ull>;

	template <typename T = f32>
	using tensor_2d_t = Kablunk::ml::tensor::Tensor<T, 2ull>;
}

#endif
