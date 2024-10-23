#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/meta/reflection/details/type_data.h"
#include "Kablunk/meta/reflection/details/type_name.h"
#include "Kablunk/Core/enum.h"
#include "Kablunk/Core/crc_32_hash.h"

#include "Kablunk/Core/concepts.hpp"

#include <string_view>
#include <typeinfo>

namespace kb::meta::refl
{ // start namespace kb::meta::refl

enum class type_t : u8
{
    // primitive types
    voidz,
    u32,
    u64,
    i32,
    i64,
    f32,
    f64,
    u16,
    i16,
    u8,
    i8,
    boolean,
    c_str,
    clazz,

    // standard types
    string_view,
    string,
    vector,
    unordered_map,
    ordered_map,
    option,
    variant,
};

namespace details
{ // start namespace ::details

// --- Reflection type to primitive type ------------------------------------------------------

// used to extract type from a type_t enum value
template <std::underlying_type_t<type_t>> struct primitive_type { using type = void; };

// Implementation details

template <> struct primitive_type<to_underlying(type_t::voidz)> { using type = void; };
template <> struct primitive_type<to_underlying(type_t::u32)> { using type = u32; };
template <> struct primitive_type<to_underlying(type_t::u64)> { using type = u64; };
template <> struct primitive_type<to_underlying(type_t::i32)> { using type = i32; };
template <> struct primitive_type<to_underlying(type_t::i64)> { using type = i64; };
template <> struct primitive_type<to_underlying(type_t::f32)> { using type = f32; };
template <> struct primitive_type<to_underlying(type_t::f64)> { using type = f64; };
template <> struct primitive_type<to_underlying(type_t::u16)> { using type = u16; };
template <> struct primitive_type<to_underlying(type_t::i16)> { using type = i16; };
template <> struct primitive_type<to_underlying(type_t::u8)> { using type = u8; };
template <> struct primitive_type<to_underlying(type_t::i8)> { using type = i8; };
template <> struct primitive_type<to_underlying(type_t::boolean)> { using type = bool; };
// template <> struct reflected_typename<to_underlying(type_t::clazz)> { using type = T; };

[[nodiscard]] consteval auto type_to_underlying_integral(type_t p_type) -> std::underlying_type_t<type_t>
{
    return static_cast<std::underlying_type_t<type_t>>(p_type);
}

#if 0
template <typename T> struct registered_type { using type = T; };

template <size_t Id>
[[nodiscard]] auto get_registered_type() -> decltype(auto)
{
    return registered_type<Id>::type;
}
#endif

// --------------------------------------------------------------------------------------------

} // end namespace ::details

namespace concepts
{ // start namespace ::concepts

template <type_t ReflT>
concept IsPrimitiveT = !std::same_as<details::primitive_type<to_underlying(ReflT)>, void>;

} // end namespace ::concepts

class refl_type
{
public:
    constexpr refl_type() noexcept = default;

    constexpr ~refl_type() noexcept = default;

    // Factory create function
    template <typename T>
    [[nodiscard]] static consteval auto create() noexcept -> refl_type;

    template <::kb::concepts::IsClassT T>
    [[nodiscard]] static consteval auto create() noexcept -> refl_type;

    // Default copy constructor
    constexpr refl_type(const refl_type&) noexcept = default;
    // Default move constructor
    constexpr refl_type(refl_type&&) noexcept = default;

    [[nodiscard]] constexpr auto operator<=>(const refl_type&) const = default;
    // Default copy assign operator
    constexpr auto operator=(const refl_type&) noexcept -> refl_type& = default;
    // Default move assign operator
    constexpr auto operator=(refl_type&&) noexcept -> refl_type& = default;

    // Check if the type is a wrapper with an underlying type
    [[nodiscard]] consteval auto is_wrapped_type() const noexcept -> bool { return m_underlying_type != type_t::voidz; }
    // Check if the type is a pointer
    [[nodiscard]] consteval auto is_pointer() const noexcept -> bool { return m_is_pointer; }
    // Check if the type is const
    [[nodiscard]] consteval auto is_const() const noexcept -> bool { return m_is_const; }

    [[nodiscard]] consteval auto get_type() const noexcept -> type_t { return m_type; }

    // Returns the underlying type of a wrapped type (vector<T> -> T, etc.) or type_t::voidz on failure
    [[nodiscard]] consteval auto get_underlying_type() const noexcept -> type_t
    {
        return m_underlying_type;
    }

    // TODO: there has to be a way to make this a member function...
#if 0
    [[nodiscard]] consteval auto get_size() const noexcept -> size_t
    {
        constexpr auto type_as_integral = static_cast<std::underlying_type_t<decltype(m_type)>>(m_type);
        return sizeof(details::reflected_typename<details::type_to_underlying_integral(m_type)>::type);
    }
#else

    template <type_t ReflT>
    [[nodiscard]] static consteval auto get_size_of_type() -> decltype(auto)
    {
        if constexpr (!std::same_as<typename details::primitive_type<to_underlying(ReflT)>::type, void>)
        {
            return sizeof(typename details::primitive_type<to_underlying(ReflT)>::type);
        }
        else
        {
            static_assert(false, "not implemented!");
            // return sizeof(typename details::registered_type<>);
        }
    }

#endif

private:
    // Private constructor with some defaults for less typing
    constexpr refl_type(
        type_t p_type,
        bool p_is_pointer,
        bool p_is_const,
        u32 p_type_id,
        type_t p_underlying_type = type_t::voidz
    ) noexcept :
        m_type{ p_type }, m_is_pointer{ p_is_pointer }, m_is_const{ p_is_const },
        m_type_id{ p_type_id }, m_underlying_type{ p_underlying_type }
    { }

private:
    // Base type
    type_t m_type;
    // Flag for whether this is a pointer to the base type
    bool m_is_pointer;
    // Flag for whether this type is const
    bool m_is_const;
    // Unique type identifier
    u32 m_type_id;
    // Optional underlying type if this is a wrapped type (vector<T>, optional<T>, etc.)
    type_t m_underlying_type;
};

static_assert(std::is_trivially_constructible_v<refl_type>);
static_assert(std::is_trivially_copy_assignable_v<refl_type>);
static_assert(std::is_trivial_v<refl_type>);


// TODO: move to implementation header
// --- IMPLEMENTATION DETAILS BELOW -----------------------------------------------------------

namespace details
{ // start namespace ::details

// --- Primitive type to reflection type ------------------------------------------------------

template <typename T>
consteval auto get_primitive_type() -> type_t;

#define KB_IMPL_DEFINE_REFL_TYPE(ttype, refltype) \
    template <> consteval auto get_primitive_type<ttype>() -> type_t { return refltype; } \
    template <> consteval auto get_primitive_type<const ttype>() -> type_t { return refltype; } \
    template <> consteval auto get_primitive_type<ttype*>() -> type_t { return refltype; } \
    template <> consteval auto get_primitive_type<const ttype*>() -> type_t { return refltype; } \

KB_IMPL_DEFINE_REFL_TYPE(u32, type_t::u32)
KB_IMPL_DEFINE_REFL_TYPE(i32, type_t::i32)
KB_IMPL_DEFINE_REFL_TYPE(u64, type_t::u64)
KB_IMPL_DEFINE_REFL_TYPE(i64, type_t::i64)
KB_IMPL_DEFINE_REFL_TYPE(f32, type_t::f32)
KB_IMPL_DEFINE_REFL_TYPE(f64, type_t::f64)
KB_IMPL_DEFINE_REFL_TYPE(u16, type_t::u16)
KB_IMPL_DEFINE_REFL_TYPE(i16, type_t::i16)
KB_IMPL_DEFINE_REFL_TYPE(u8, type_t::u8)
KB_IMPL_DEFINE_REFL_TYPE(i8, type_t::i8)
KB_IMPL_DEFINE_REFL_TYPE(bool, type_t::boolean)

template <> consteval auto get_primitive_type<const char*>() -> type_t { return type_t::c_str; }

template <::kb::concepts::IsClassT T>
consteval auto get_primitive_type() -> type_t { return type_t::clazz; }

// --------------------------------------------------------------------------------------------

} // end namespace ::details

// private macro, not for public use
#define KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(ttype) \
    template <> \
    consteval auto refl_type::create<ttype>() noexcept -> refl_type \
    { \
        return { details::get_primitive_type<std::remove_pointer<ttype>::type>(), \
            std::is_pointer<ttype>::value, false, \
            COMPILE_TIME_CRC32_STR(::kb::meta::refl::details::type_name<ttype>::value.data()) \
        }; \
    } \
    template <> \
    consteval auto refl_type::create<const ttype>() noexcept -> refl_type \
    { \
        return { details::get_primitive_type<std::remove_pointer<const ttype>::type>(), \
            std::is_pointer<ttype>::value, true, \
            COMPILE_TIME_CRC32_STR(::kb::meta::refl::details::type_name<ttype>::value.data()) \
        }; \
    }

KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u32);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u32*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u64);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u64*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i32);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i32*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i64);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i64*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(f32);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(f32*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(f64);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(f64*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u16);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u16*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i16);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i16*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u8);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(u8*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i8);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(i8*);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(bool);
KB_IMPL_TYPE_DECLARATION_CREATE_FUNC_DECL(bool*);

template <>
consteval auto refl_type::create<const char*>() noexcept -> refl_type
{
    return refl_type{
        type_t::c_str,
        true,
        true,
        COMPILE_TIME_CRC32_STR(details::type_name<const char*>::value.data())
    };
}

// TODO: figure out
#if 0
template <>
consteval auto refl_type::create<std::vector<T>>() noexcept -> refl_type
{
    return { type_t::vector, false, false };
}
#endif

template <::kb::concepts::IsClassT T>
consteval auto refl_type::create() noexcept -> refl_type
{
    return refl_type{
        type_t::clazz,
        std::is_pointer_v<T>,
        std::is_const_v<T>,
        COMPILE_TIME_CRC32_STR(details::type_name<T>::value.data())
    };
}


} // end namespace kb::meta::refl
