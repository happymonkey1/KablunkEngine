#pragma once

#include "Kablunk/Utilities/Containers/Node.hpp"
#include <type_traits>

namespace Kablunk::Utilties
{
	// default function template parameter
	using void_compare_func_t = void(*)(void*, void*);

	template <typename T, typename CompareFunc = void_compare_func_t>
	class LinkedList
	{
	public:
		class Iterator
		{
		public:
			Iterator(ListNode<T>* ptr) : m_ptr{ ptr } {}

			T& operator*() { return m_ptr->data; }
			T* operator->() { return &(m_ptr->data); }

			Iterator& operator++()
			{
				if (m_ptr)
					m_ptr = m_ptr->next;

				return *this;
			}

			bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
			bool operator!=(const Iterator& other) const { return !(*this == other); }
		private:
			ListNode<T>* m_ptr;
		};

		class ConstIterator
		{
		public:
			ConstIterator(ListNode<T>* ptr) : m_ptr{ ptr } {}

			T& operator*() const { return *m_ptr; }
			T* operator->() const { return m_ptr->data; }

			ConstIterator& operator++()
			{
				if (m_ptr)
					m_ptr = m_ptr->next;

				return *this;
			}

			bool operator==(const ConstIterator& other) const { return m_ptr == other.m_ptr; }
			bool operator!=(const ConstIterator& other) const { return !(*this == other); }
		private:
			ListNode<T> m_ptr;
		};
	public:
		LinkedList() = default;
		LinkedList(CompareFunc compare_func) : m_compare_func{ compare_func } {}
		LinkedList(const LinkedList&);
		LinkedList(LinkedList&&) noexcept;
		~LinkedList();

		void clear();

		T& Front() { return m_root->data; }
		const T& Front() const { return m_root->data; }

		T& Back() { return m_end->data; }
		const T& Back() const { return m_end->data; }

		// Unsorted
		template <
			typename Q = CompareFunc,
			std::enable_if_t<std::is_same<Q, void_compare_func_t>::value>* = nullptr
		>
		void Insert(const T& val)
		{
			if (m_root)
			{
				m_end->next = new ListNode<T>{ val };
				m_end = m_end->next;
			}
			else
			{
				m_root = new ListNode<T>{ val };
				m_end = m_root;
			}
			
			m_size++;
		}

		// Sorted
		template <
			typename Q = CompareFunc,
			std::enable_if_t<!std::is_same<Q, void_compare_func_t>::value>* = nullptr
		>
		void Insert(const T& val)
		{
			ListNode<T>* cur = m_root;
			ListNode<T>* prev = nullptr;
			while (cur && m_compare_func(cur->data, val))
			{
				prev = cur;
				cur = cur->next;
			}

			if (prev)
			{
				prev->next = new ListNode<T>{ val, cur };
				// update end ptr if cur is nullptr
				if (!cur)
					m_end = prev->next;
			}
			else
				m_root = new ListNode<T>{ val, cur };

			m_size++;
		}

		void Remove(const T& val);
		void RemoveIndex(size_t index);
		void RemoveFront();
		void RemoveBack();

		ListNode<T>* Find(const T& val) const;

		bool contains(const T& val) const { return Find(val) ? true : false; }
		size_t size() const { return m_size; }

		Iterator begin() { return Iterator{ m_root }; }
		Iterator end() { return Iterator{ nullptr }; }

		ConstIterator cbegin() const { return ConstIterator{ m_root }; }
		ConstIterator cend() const { return ConstIterator{ nullptr }; }

		T& operator[](size_t index);
		const T& operator[](size_t index) const;
	private:
		ListNode<T>* m_root = nullptr;
		ListNode<T>* m_end = nullptr;
		size_t m_size = 0;
		CompareFunc m_compare_func;
	};

	template <typename T, typename CompareFunc /*= void*/>
	void LinkedList<T, CompareFunc>::clear()
	{
		ListNode<T>* cur = m_root;
		ListNode<T>* next;
		while (cur)
		{
			next = cur->next;
			delete cur;
			cur = next;
		}
	}

	template <typename T, typename CompareFunc>
	LinkedList<T, CompareFunc>::LinkedList(const LinkedList& other)
	{
		ListNode<T>* cur = other.m_root;
		ListNode<T>* m_root = new ListNode<T>{ *other.m_root };
		ListNode<T>* head_ptr = m_root;

		while (cur)
		{
			m_root->next = new ListNode<T>{ *cur };
			cur = cur->next;
			m_root = m_root->next;
		}

		m_end = m_root;
		m_root = head_ptr;
		m_size = other.m_size;
	}

	template <typename T, typename CompareFunc>
	LinkedList<T, CompareFunc>::LinkedList(LinkedList&& other) noexcept
		: m_root{ other.m_root }, m_end{ other.m_end }, m_size{ other.m_size }, m_compare_func{ other.m_compare_func }
	{
		other.m_root = nullptr;
		other.m_end = nullptr;
		other.m_size = 0;
		other.m_compare_func = nullptr;
	}

	template <typename T, typename CompareFunc>
	LinkedList<T, CompareFunc>::~LinkedList()
	{
		clear();
	}

	template <typename T, typename CompareFunc>
	void LinkedList<T, CompareFunc>::Remove(const T& val)
	{
		ListNode<T>* cur = m_root;
		ListNode<T>* prev = nullptr;
		while (cur)
		{
			if (*cur == val)
			{
				// if we are not at the root, continue. Otherwise, update root.
				if (prev)
				{
					// if we are not at the end, update node. Otherwise update end.
					if (cur->next)
						prev->next = cur->next;
					else
					{
						m_end = prev;
						prev->next = nullptr;
					}
				}
				else
					m_root = cur->next;

				delete cur;
				m_size--;
				break;
			}

			prev = cur;
			cur = cur->next;
		}
	}

	template <typename T, typename CompareFunc>
	ListNode<T>* LinkedList<T, CompareFunc>::Find(const T& val) const
	{
		// #TODO implement SortedFind which should be O(log n)

		ListNode<T>* cur = m_root;
		while (cur)
		{
			if (*cur == val)
				return cur;

			cur = cur->next;
		}

		return nullptr;
	}

	template <typename T, typename CompareFunc>
	void LinkedList<T, CompareFunc>::RemoveIndex(size_t index)
	{
		KB_CORE_ASSERT(index < m_size, "out of bounds!");
		size_t counter = 0;
		ListNode<T>* cur = m_root;
		ListNode<T>* prev = nullptr;
		while (cur)
		{
			if (counter++ == index)
			{
				// check if we are at root, and update root accordingly
				if (prev)
				{
					// check if we are at end, and update end accordingly
					if (cur->next)
						prev->next = cur->next;
					else
					{
						prev->next = nullptr;
						m_end = prev;
					}
				}
				else
					m_root = cur->next;

				break;
			}

			prev = cur;
			cur = cur->next;
		}

		delete cur;
		m_size--;
	}

	template <typename T, typename CompareFunc>
	void LinkedList<T, CompareFunc>::RemoveBack()
	{
		// #TODO this is O(n), might be able to be faster?
		RemoveIndex(m_size - 1);
	}

	template <typename T, typename CompareFunc>
	void LinkedList<T, CompareFunc>::RemoveFront()
	{
		if (!m_root)
			return;

		ListNode<T>* cur = m_root;
		m_root = m_root->next;

		delete cur;
		m_size--;
	}

	template <typename T, typename CompareFunc>
	T& LinkedList<T, CompareFunc>::operator[](size_t index)
	{
		KB_CORE_ASSERT(index < m_size, "out of bounds!");
		
		size_t counter = 0;
		ListNode<T>* cur = m_root;
		while (cur)
		{
			if (counter++ == index)
				break;

			cur = cur->next;
		}

		return *cur;
	}

	template <typename T, typename CompareFunc>
	const T& LinkedList<T, CompareFunc>::operator[](size_t index) const
	{
		KB_CORE_ASSERT(index < m_size, "out of bounds!");

		size_t counter = 0;
		ListNode<T>* cur = m_root;
		while (cur)
		{
			if (counter++ == index)
				break;

			cur = cur->next;
		}

		return *cur;
	}

}
