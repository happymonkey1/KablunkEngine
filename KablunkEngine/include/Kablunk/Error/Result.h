#pragma once

#ifndef KABLUNK_ERROR_RESULT_H
#define KABLUNK_ERROR_RESULT_H

#include <variant>

namespace kb::error
{

	// Struct that holds a variant of okay and error results from a function. Modeled after rust's error handling system
	template <typename T, typename Err>
	class Result
	{

		template <typename FuncT>
		Result& on_error(FuncT&& func) noexcept
		{
			if (std::holds_alternative<Err>(m_result))
				func(std::get<Err>(m_result));

			return *this;
		}

		T& get() 
		{ 
			if (std::holds_alternative<T>(m_result))
				return std::get<T>(m_result);
			else
				static_assert(false, "on_error() was not called!");
		}
	private:
		std::variant<T, Err> m_result;
	};

}

#endif
