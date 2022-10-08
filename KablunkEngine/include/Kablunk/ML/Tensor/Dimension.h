#pragma once
#ifndef KABLUNK_ML_TENSOR_DIMENSION_H
#define KABLUNK_ML_TENSOR_DIMENSION_H

// #TODO this is so we can use fmt::format, replace with correct include
#include <spdlog/spdlog.h>

namespace Kablunk::ml::tensor
{

	// #TODO replace with stack allocated dimension class
	template <size_t rank>
	using dimension_t = std::array<size_t, rank>;

	template <size_t rank>
	inline std::string dimension_to_string(dimension_t<rank> dimensions)
	{
		std::string out = "(";
		for (size_t i = 0; i < dimensions.size(); ++i)
		{
			out += std::to_string(dimensions[i]);
			if (i < dimensions.size() - 1)
				out += ", ";
		}

		out += ")";
		return out;
	}

}

#endif
