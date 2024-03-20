#pragma once

#include <tuple>
#include <type_traits>

namespace kb::meta
{

template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T>
struct tuple_has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct tuple_has_type<T, std::tuple<U, Ts...>> : tuple_has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct tuple_has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <class T, class Tuple>
struct tuple_index;

template <class T, class... Types>
struct tuple_index<T, std::tuple<T, Types...>>
{
    static constexpr std::size_t value = 0;
};

template <class T, class U, class... Types>
struct tuple_index<T, std::tuple<U, Types...>>
{
    static constexpr std::size_t value = 1 + tuple_index<T, std::tuple<Types...>>::value;
};

template <typename T>
struct tuple_remove_first_type{};

template <typename T, typename... Types>
struct tuple_remove_first_type<std::tuple<T, Types...>>
{
    using type = std::tuple<Types...>;
};
}
