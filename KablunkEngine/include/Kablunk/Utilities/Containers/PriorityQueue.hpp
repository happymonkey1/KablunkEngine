#pragma once

#include "Kablunk/Utilities/Containers/LinkedList.hpp"

#include <algorithm>
#include <type_traits>

namespace Kablunk::Utilties
{
	// #TODO extend so priority queue can use multiple different containers, like max heap, min heap (like std priority queue container)
	template <
		typename T, 
		typename CompareFunc = std::greater<T>
	>
	class PriorityQueue
	{
	public:
		PriorityQueue() = default;
		PriorityQueue(CompareFunc compare_func) : m_list{ compare_func } {}
		PriorityQueue(const PriorityQueue& other) : m_list{ other.m_list } {}
		PriorityQueue(PriorityQueue&& other) noexcept : m_list{ std::move(other.m_list) } { other.m_list = {}; }
		~PriorityQueue() = default;

		void Push(const T& val) { m_list.Insert(val); }

		void PopFront() { m_list.RemoveFront(); };
		void PopBack() { m_list.RemoveBack(); };

		T& Front() { return m_list.Front(); }
		const T& Front() const { return m_list.Front(); }
		
		T& Back() { return m_list.Back(); }
		const T& Back() const { return m_list.Back(); }

		size_t size() const { return m_list.size(); }

		auto begin() { return m_list.begin(); }
		auto end() { return m_list.end(); }

		bool contains(const T& val) const { return m_list.contains(val); }
	private:
		LinkedList<T, CompareFunc> m_list;
	};
}