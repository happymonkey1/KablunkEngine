#pragma once
#ifndef KABLUNK_ML_TENSOR_TENSOR_BASE_H
#define KABLUNK_ML_TENSOR_TENSOR_BASE_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/ML/Tensor/Dimension.h"

#include <string>
#include <numeric>

namespace Kablunk::ml::tensor
{

	// #TODO fixed size (stack allocated) tensor storage

	// heap allocated tensor storage
	template <typename T, size_t rank>
	class TensorBuffer
	{
	public:
		using value_t = T;
		using indices_t = dimension_t<rank>;
	public:
		TensorBuffer() : m_dimensions{}, m_data{ nullptr } { }

		// variadic template for creating a TensorBuffer from arguments
		template <typename... dimensions_t>
		TensorBuffer(dimensions_t... dimensions) 
			: m_dimensions{ dimension_t<rank>{ static_cast<size_t>(dimensions)... }}, m_data{nullptr}, m_size{0}
		{
			// ensure passed in dimensions matches specified rank
			KB_CORE_ASSERT(rank == sizeof...(dimensions), "[TensorStorage] Passed in dimensions must match specified rank!");

			m_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1ull, std::multiplies<size_t>());
			m_data = new value_t[m_size]{};
		}

		TensorBuffer(const dimension_t<rank>& dimensions) 
			: m_dimensions{ dimensions }, m_data{ nullptr }, m_size{ 0 }
		{
			// ensure passed in dimensions matches specified rank
			KB_CORE_ASSERT(rank == m_dimensions.size(), "[TensorStorage] Passed in dimensions must match specified rank!");

			m_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1ull, std::multiplies<size_t>());
			m_data = new value_t[m_size]{};
		}

		TensorBuffer(const TensorBuffer& other)
			: m_dimensions{ other.m_dimensions }, m_data{ nullptr }
		{
			m_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1ull, std::multiplies<size_t>());
			size_t buf_size = m_size;
			m_data = new value_t[buf_size];

			// copy data over
			for (size_t i = 0; i < buf_size; ++i)
				m_data[i] = other.m_data[i];
		}

		TensorBuffer(TensorBuffer&& other)
			: m_dimensions{ other.m_dimensions }, m_data{ other.m_data }, m_size{ other.m_size }
		{
			other.m_dimensions = 0;
			other.m_size = 0;
			other.m_data = nullptr;
		}

		~TensorBuffer()
		{
			release();
		}

		// get pointer to the raw buffer
		value_t* get() { return m_data; }
		const value_t* get() const { return m_data; }

		// get specific dimension 
		// #TODO overflow protection
		size_t get_dimension(size_t index) const noexcept { return m_dimensions[index]; }

		// index into the buffer
		const value_t* at(const indices_t& indices) const
		{
			KB_CORE_ASSERT(indices.size() == rank, "[TensorBuffer] Passed in indices do not match specified rank!");
			KB_CORE_ASSERT(indices.size() == m_dimensions.size(), "[TensorBuffer] Passed in indices do not match specified dimensions!");

			size_t out_index = 0;
			for (size_t i = 0; i < indices.size(); ++i)
			{
				size_t next_dimension = 1ull;
				if (i < indices.size() - 1)
					next_dimension = m_dimensions[i + 1];

				out_index += indices[i] * next_dimension;
			}

			return m_data + out_index;
		}

		value_t* at(const indices_t& indices)
		{
			KB_CORE_ASSERT(indices.size() == rank, "[TensorBuffer] Passed in indices do not match specified rank!");

			size_t out_index = 0;
			for (size_t i = 0; i < indices.size(); ++i)
			{
				size_t next_dimension = 1ull;
				if (i < indices.size() - 1)
					next_dimension = m_dimensions[i + 1];

				out_index += indices[i] * next_dimension;
			}

			return m_data + out_index;
		}

		// get a string representation of the storage buffer
		std::string to_string() const
		{
			// #TODO make pretty
			KB_CORE_ASSERT(m_dimensions.size() <= 2, "printing out tensors only supported for rank <= 2!");
			std::string out = "[";
			for (size_t i = 0; i < m_size; ++i)
			{
				out += std::to_string(m_data[i]);
				if (i < m_size - 1)
					out += ", ";
			}

			out += "]";
			return out;
		}

		// get size of the buffer
		size_t size() const { return m_size; }

		// check whether the buffer is valid
		bool is_valid() const { return m_data != nullptr; }

		// release allocated memory
		void release()
		{
			if (m_data)
				delete[] m_data;
			else
				KB_CORE_WARN("[TensorStorage] tried deleting TensorStorage with null data?");

			m_size = 0;
			m_data = nullptr;
		}

		void resize()
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		const dimension_t<rank>& get_dimensions() const { return m_dimensions; }

		// =========
		// operators
		// =========

		TensorBuffer& operator=(const TensorBuffer& other)
		{
			// compare by pointer since we don't need to have comparison operator
			// when are the pointers ever equivalent?
			if (this != &other)
			{
				TensorBuffer temp{ other };
				swap(temp);
			}

			return *this;
		}

		TensorBuffer& operator=(TensorBuffer&& other)
		{
			std::swap(m_data, other.m_data);
			std::swap(m_dimensions, other.m_dimensions);
			std::swap(m_size, other.m_size);

			other.m_data = nullptr;
			
			return *this;
		}
		
		// index the raw buffer using element offset (index position)
		value_t& operator[](size_t index) { return m_data[index]; }
		const value_t& operator[](size_t index) const { return m_data[index]; }

		// index the raw buffer using indices
		value_t& operator[](const indices_t& indices)
		{
			return *at(indices);
		}

		const value_t& operator[](const indices_t& indices) const
		{
			return *at(indices);
		}
	private:
		// helper function to swap underlying data between two tensor buffers
		void swap(TensorBuffer& other)
		{
			T* tmp = m_data;
			m_data = other.m_data;
			other.m_data = tmp;

			dimension_t<rank> dim_tmp = m_dimensions;
			m_dimensions = other.m_dimensions;
			other.m_dimensions = dim_tmp;
		}

	private:
		// dimensions of the storage
		dimension_t<rank> m_dimensions;
		// cached element count
		size_t m_size = 0;
		// block of data
		value_t* m_data = nullptr;
	};

}

#endif
