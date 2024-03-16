#pragma once

namespace kb::meta
{ // start namespace kb::meta

template <typename FuncT, typename Arg>
auto invoke_func(
    FuncT p_func,
    Arg&& p_arg
) -> decltype(auto)
{
    return p_func(std::forward<Arg>(p_arg));
}

namespace details
{ // start namespace ::details

// call a function with a tuple argument list
// unpacks tuple argument list into arguments with template magic
template <typename FuncT, typename... Args, std::size_t... Indices>
auto invoke_func_helper(
    FuncT p_func,
    std::tuple<Args...>&& p_args,
    std::index_sequence<Indices...>
) -> decltype(auto)
{
    return p_func(std::get<Indices>(p_args)...);
}
} // end namespace ::details

template <typename FuncT, typename... Args>
auto invoke_func(
    FuncT p_func,
    std::tuple<Args...> p_args
) -> decltype(auto)
{
    return details::invoke_func_helper(
        p_func,
        std::forward<std::tuple<Args...>>(p_args),
        std::index_sequence_for<Args...>{}
    );
}
} // end namespace kb::meta
