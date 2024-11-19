#pragma once

#include "Kablunk/meta/reflection/type.h"

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/util.h"

namespace kb::meta::refl
{ // start namespace kb::meta::refl

namespace details
{ // start namespace ::details

template <type_t ReflT, bool IsPointer>
[[nodiscard]] constexpr auto constexpr_refl_cast_impl(
    const void* p_data
) noexcept -> decltype(auto);

#define KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(refl_type, cast_type) \
    template <> constexpr inline auto constexpr_refl_cast_impl<refl_type, false>(const void* p_data) noexcept -> decltype(auto) \
    { \
        return *static_cast<const cast_type*>(p_data); \
    }

#define KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER(refl_type, cast_type) \
    template <> constexpr inline auto constexpr_refl_cast_impl<refl_type, true>(const void* p_data) noexcept -> decltype(auto) \
    { \
        return static_cast<const cast_type*>(p_data); \
    }

KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::u32, u32);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::u32, u32);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::u64, u64);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::u64, u64);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::i32, i32);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::i32, i32);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::i64, i64);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::i64, i64);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::f32, f32);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::f32, f32);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::f64, f64);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::f64, f64);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::u16, u16);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::u16, u16);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::i16, i16);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::i16, i16);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::u8, u8);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::u8, u8);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::i8, i8);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::i8, i8);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::boolean, bool);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::boolean, bool);
// c_str can only be pointer
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::c_str, char);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::string, std::string);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::string, std::string);
KB_IMPL_REFL_CONSTEXPR_CAST_NOT_POINTER(type_t::string_view, std::string_view);
KB_IMPL_REFL_CONSTEXPR_CAST_IS_POINTER( type_t::string_view, std::string_view);

} // end namespace ::details

#if 0
template <>
constexpr auto constexpr_refl_cast(
    const void* p_data
) noexcept -> decltype(auto)
{
    constexpr auto type = p_refl_type.get_type();
    if constexpr (type == type_t::voidz)
    {
        static_assert("Cannot cast data to void!");
        unreachable();
    }
    else if constexpr (type == type_t::u32)
    {
        
    }
    else if constexpr (type == type_t::u64)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const u64*>(p_data); }
        else { return static_cast<const u64*>(p_data); }
    }
    if constexpr (type == type_t::i32)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const i32*>(p_data); }
        else { return static_cast<const i32*>(p_data); }
    }
    else if constexpr (type == type_t::i64)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const i64*>(p_data); }
        else { return static_cast<const i64*>(p_data); }
    }
    if constexpr (type == type_t::f32)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const f32*>(p_data); }
        else { return static_cast<const f32*>(p_data); }
    }
    else if constexpr (type == type_t::f64)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const f64*>(p_data); }
        else { return static_cast<const f64*>(p_data); }
    }
    else if constexpr (type == type_t::u16)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const u16*>(p_data); }
        else { return static_cast<const u16*>(p_data); }
    }
    else if constexpr (type == type_t::i16)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const i16*>(p_data); }
        else { return static_cast<const i16*>(p_data); }
    }
    else if constexpr (type == type_t::u8)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const u8*>(p_data); }
        else { return static_cast<const u8*>(p_data); }
    }
    else if constexpr (type == type_t::i8)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const i8*>(p_data); }
        else { return static_cast<const i8*>(p_data); }
    }
    else if constexpr (type == type_t::boolean)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const bool*>(p_data); }
        else { return static_cast<const bool*>(p_data); }
    }
    else if constexpr (type == type_t::c_str)
    {
        static_assert(p_refl_type.is_pointer());
        return static_cast<const char*>(p_data);
    }
    else if constexpr (type == type_t::clazz)
    {
        KB_STATIC_ASSERT(false, "class reflection casting is not implemented!");
    }
    else if constexpr (type == type_t::string_view)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const std::string_view*>(p_data); }
        else { return static_cast<const std::string_view*>(p_data); }
    }
    else if constexpr (type == type_t::string)
    {
        if constexpr (!p_refl_type.is_pointer()) { return *static_cast<const std::string*>(p_data); }
        else { return static_cast<const std::string*>(p_data); }
    }
    else if constexpr (type == type_t::vector)
    {
        KB_STATIC_ASSERT(false, "vector reflection casting is not implemented!");
    }
    else if constexpr (type == type_t::unordered_map)
    {
        KB_STATIC_ASSERT(false, "unordered_map reflection casting is not implemented!");
    }
    else if constexpr (type == type_t::ordered_map)
    {
        KB_STATIC_ASSERT(false, "ordered_map reflection casting is not implemented!");
    }
    else if constexpr (type == type_t::option)
    {
        KB_STATIC_ASSERT(false, "option reflection casting is not implemented!");
    }
    else if constexpr (type == type_t::variant)
    {
        KB_STATIC_ASSERT(false, "variant reflection casting is not implemented!");
    }
    else
    {
        KB_STATIC_ASSERT(false, "unhandled reflection type!");
    }

    unreachable();
}
#endif

} // end namespace kb::meta::refl
