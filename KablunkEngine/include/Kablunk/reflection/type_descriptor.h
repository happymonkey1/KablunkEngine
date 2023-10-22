#pragma once

#include <array>
#include <optional>
#include <type_traits>
#include <tuple>
#include <variant>

namespace kb::refl
{ // start namespace kb::refl

template <auto... member_pointers>
struct member_list
{
public:
    // tuple pack alias of member pointer template argument pack
    using tuple_t = decltype(std::tuple(member_pointers));
private:
    // type alias for the member type of a specific index in the tuple member pointer template argument pack
    template <size_t k_member_index>
    using member_type_t = typename member_pointer::return_type<std::remove_cvref_t<decltype(std::get<k_member_index>(tuple_t()))>>::type;

    // type alias for a pointer to a member type
    template <typename member_ptr_t>
    using member_ptr_type_t = typename member_pointer::return_type<std::remove_cvref_t<member_ptr_t>>::type;
public:
    // type alias for a variant containing all non-void types in the template argument pack
    using type_variant_t = std::variant<filter_void_t<member_ptr_type_t<decltype(member_pointers)>>...>;
public:
    // gets the count of members
    static constexpr const auto count() -> size_t { return sizeof ... (member_pointers); }

    // apply a function to a variadic pack of the template argument pack on a mutable reference 
    // to an instance of the reflected type
    template <typename reflected_t, typename func_t>
    static constexpr auto apply(func_t p_func, reflected_t& p_reflected) -> decltype(auto)
    {
        return p_func(std::forward<member_pointers&&>(p_reflected.*member_pointers)...);
    }

    // apply a function to a variadic pack of the template argument pack on an immutable reference 
    // to an instance of the reflected type
    template <typename reflected_t, typename func_t>
    static constexpr auto apply(func_t p_func, const reflected_t& p_reflected) -> decltype(auto)
    {
        return p_func(std::forward<member_pointers&&>(p_reflected.*member_pointers)...);
    }

    // apply a function to member variables of a variadic pack of the template argument pack 
    // on a mutable reference to the object
    template <typename reflected_t, typename func_t>
    static constexpr auto apply(func_t p_func, const reflected_t& p_reflected) -> decltype(auto)
    {
        // #TODO can we do perfect forwarding here?
        return (apply_if_not_member_function_impl(p_func, member_pointers, p_reflected), ...);
    }

    // apply a function to a default initialized instance of the reflected type, for each member variable.
    // this does not require an instance of the object to be passed in
    template <typename func_t>
    static constexpr auto apply_to_static_type(func_t p_func) -> decltype(auto)
    {
        // #TODO can we do perfect forwarding here?
        return func_t(member_pointers...);
    }
private:
    template <typename func_t, typename member_ptr_t, typename reflected_t>
    static constexpr auto apply_if_not_member_function_impl(
        func_t p_func, 
        member_ptr_t p_member, 
        reflected_t& p_reflected
    ) -> decltype(auto)
    {
        if constexpr (!std::is_member_function_pointer_v<decltype(p_member)>)
            p_func(p_reflected.*member)
    }

    // apply a function to a member variable of an instance of the reflected type
    template <typename func_t, typename reflected_t>
    static constexpr auto apply_to_member(
        size_t p_member_index, 
        func_t&& p_func, 
        reflected_t&& p_reflected
    ) -> decltype(auto)
    {
        size_t member_count = 0;

        auto unwrap_with_counter = [&member_count, &p_func, p_member_index]() -> decltype(auto)
            {
                auto unwrap = [member_count, &p_func, p_member_index](auto& member) -> decltype(auto)
                    {
                        if (member_count == p_member_index)
                            p_func(member)
                    };

                ++member_count;

                return unwrap;
            };

        // i have no idea wtf this syntax of function call is / what it is *actually* doing...
        (apply_if_not_member_function_impl(unwrap_with_counter(), member_pointers, std::forward<reflected_t>(p_obj)), ...);
    }

    template <size_t k_member_index, typename func_t, typename reflected_t>
    static constexpr auto apply_to_member(func_t&& p_func, reflected_t&& p_reflected) -> decltype(auto)
    {
        // #TODO can we do perfect forwarding here?
        p_func(p_reflected.*ttype::nth_element<k_member_index>(member_pointers...))
    }
};

} // end namespace kb::refl
