#pragma once

#include <Kablunk/Core/CoreTypes.h>

#include <type_traits>

// reference https://github.com/rpclib/rpclib/blob/master/include/rpc/detail/func_traits.h
namespace kb::meta
{ // start namespace kb::meta

template <typename T, T Value>
struct constant : std::integral_constant<T, Value>{};

template <bool Value>
using bool_trait = constant<bool, Value>;

using true_trait = bool_trait<true>;
using false_trait = bool_trait<false>;

template <u32 N>
using is_zero_func_args = std::conditional_t<
    N == 0,
    true_trait,
    false_trait
>;

template <u32 N, typename... Types>
using nth_func_type = std::tuple_element_t<N, std::tuple<Types...>>;

// args trait meta programming

struct zero_args{};
struct non_zero_args{};


template <u32 N>
struct arg_count_trait
{
    using type = zero_args;
};

template <>
struct arg_count_trait<0>
{
    using type = non_zero_args;
};

// return trait meta programming

struct void_result {};
struct non_void_result {};

template <typename T>
struct result_trait
{
    using type = non_void_result;
};

template <>
struct result_trait<void>
{
    using type = void_result;
};

// meta programming to determine function argument types, etc.

// func trait specialization for operator() overload (lambda, etc.)
template <typename T>
struct func_traits : func_traits<decltype(&T::operator())> {};

// func trait specialization for member function
template <typename ClassT, typename ReturnT, typename... Args>
struct func_traits<ReturnT(ClassT::*)(Args...)> : func_traits<ReturnT (*)(Args...)> {};

// func trait specialization for member function
template <typename ClassT, typename ReturnT, typename... Args>
struct func_traits<ReturnT(ClassT::*)(Args...) const> : func_traits<ReturnT(*)(Args...)> {};

// func trait specialization for function pointer
template <typename ReturnT, typename... Args>
struct func_traits<ReturnT(*)(Args...)>
{
    using return_type_t = ReturnT;
    using arg_count = std::integral_constant<std::size_t, sizeof...(Args)>;
    using args_type_t = std::tuple<std::decay_t<Args>...>;
};

// helper meta function to determine if the return type is void
template <typename FuncT>
using is_void_return_type = std::is_void<typename func_traits<FuncT>::return_type_t>;

template <typename FuncT>
using is_single_arg = std::conditional_t<
    func_traits<FuncT>::arg_count == 1,
    true_trait,
    false_trait
>;

} // end namespace kb::meta
