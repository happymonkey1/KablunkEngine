#pragma once

namespace kb::reflect
{ // start namespace kb::reflect

enum class underlying_type_t
{
    // signed integers
    int8,
    int16,
    int32,
    int64,

    // unsigned integers
    uint8,
    uint16,
    uint32,
    uint64,

    // floats
    float32,
    float64,

    // strings & characters
    character,
    cpp_string,

    // boolean
    boolean,

    // std types
    std_vector,
    std_map,
    std_unordered_map,

    // user defined types
    user_struct,
    user_class,

    // function
    function,

    // math
    vec2,
    vec3,
    vec4,
    mat4,

    // enums
    c_enum,
    enum_class,

    // undefined
    UNDEFINED,

    // invalid
    NONE
};

// header which contains data about the reflected type
struct type_trait_t
{
    // underlying type enum
    underlying_type_t m_type = underlying_type_t::NONE;
    // flag for whether the data is const
    bool m_is_const = false;
    // flag for whether the data is a pointer
    bool m_is_ptr = false;
};

// header which contains reflected data about a member
struct member_t
{
    // type information
    type_trait_t m_type_traits{};
    // pointer to the head of the block
    void* m_head = nullptr;
    // size of the member
    size_t m_size = 0;
    // member offset
    size_t m_offset = 0;
    // name of the member
    const char* m_name = nullptr;
};

} // end namespace kb::reflect
