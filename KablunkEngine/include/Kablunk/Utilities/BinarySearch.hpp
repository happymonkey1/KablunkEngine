#pragma once

#include <vector>
#include <stdint.h>

namespace kb::Utilties
{

	template <typename T, typename CompareFunc, typename iterator = std::vector<T>::const_iterator>
	iterator BinarySearch(const std::vector<T>& list, const T& search_val, CompareFunc compare, size_t lower_bound = 0, size_t upper_bound = UINT64_MAX)
	{
		upper_bound = upper_bound == UINT64_MAX ? list.size() - 1 : upper_bound;
		if (upper_bound < lower_bound)
			return list.end();

		size_t mid = lower_bound + (upper_bound - lower_bound) / 2u;
		int8_t i = compare(search_val, list[mid]);
		if (i == 0)
			return list.begin() + mid;
		else if (i > 0)
			return BinarySearch(list, search_val, compare, lower_bound, mid - 1);
		else
			return BinarySearch(list, search_val, compare, mid + 1, upper_bound);
	}


}
