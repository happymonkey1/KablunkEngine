#pragma once

#include <type_traits>

namespace kb::Utilties
{
	template <typename T>
	struct ListNode
	{
		T data;
		ListNode* next = nullptr;

		ListNode(const T& val, ListNode* next_node = nullptr) : data{ val }, next{ next_node } {}
		ListNode(T&& val, ListNode* next_node = nullptr) : data{ &std::move(val) }, next{next_node} {}

		T& operator*() { KB_CORE_ASSERT(data != nullptr); return *data; }
		const T& operator*() const { KB_CORE_ASSERT(data != nullptr); return *data; }
		T* operator->() { return data; }

		bool operator==(const ListNode* other) const { return data == other->data && next == other->next; }
		bool operator!=(const ListNode* other) const { return !(*this == other); }
	};

	template <typename T>
	struct TreeNode
	{
		T data;
		TreeNode* left = nullptr;
		TreeNode* right = nullptr;
	};
}
