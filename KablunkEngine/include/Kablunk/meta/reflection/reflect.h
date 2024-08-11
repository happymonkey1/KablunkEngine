#pragma once

#include "Kablunk/meta/reflection/type.h"
#include "Kablunk/meta/reflection/member.h"
#include "Kablunk/meta/reflection/class.h"
#include "Kablunk/meta/reflection/cast.h"

#include <cstddef>

namespace kb::meta::refl
{ // start namespace kb::meta::refl


#define KB_REFLECT_CLASS_START(type) \
    using reflected_class_type_t = type; \
    [[nodiscard]] static consteval auto get_reflected_class() noexcept -> decltype(auto) { \
        using clazz = type; \
        return ::kb::meta::refl::reflect_class( \
            #type, \
            sizeof(clazz),

#define KB_REFLECT_MEMBER(member) \
            ::kb::meta::refl::refl_member::create<decltype(member)>(#member, offsetof(clazz, member))

#define KB_REFLECT_CLASS_END \
        ); }

#define KB_REFL_CONSTEXPR_CAST(refl_type_obj, data_ptr) \
    ::kb::meta::refl::details::constexpr_refl_cast_impl<((refl_type_obj).get_type()), ((refl_type_obj).is_pointer())>(static_cast<const void*>((data_ptr)))

#define KB_REFL_TYPE_SIZE(refl_type_obj) \
    ::kb::meta::refl::refl_type::get_size_of_type<(refl_type_obj).get_type()>()

// Public api for reflecting a class
template <typename... Args>
[[nodiscard]] consteval auto reflect_class(
    std::string_view p_class_name,
    u32 p_size,
    Args&&... p_members
) noexcept -> refl_class<sizeof...(Args)>
{
    return refl_class{
        p_class_name,
        std::array<refl_member, sizeof...(p_members)>{ std::forward<Args>(p_members)... },
        p_size
    };
}

} // end namespace kb::meta::refl
