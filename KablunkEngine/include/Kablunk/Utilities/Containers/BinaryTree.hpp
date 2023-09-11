#pragma once

#include <Kablunk/Utilities/Containers/Node.hpp>

namespace kb::Utilties
{
	template <typename T>
	class BinaryTree
	{
	public:
		using CompareFunc = bool(*)();
	public:
		BinaryTree() = default;

		void Insert();
		void Remove(const T& );

	private:
		TreeNode<T>* m_root;
		CompareFunc m_compare_func = [](const T& a, const T& b) { return a < b; };
	};

}
