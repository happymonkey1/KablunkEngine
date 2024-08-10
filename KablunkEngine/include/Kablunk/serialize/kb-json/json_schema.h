#pragma once

#include <variant>

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/coroutine/generator.h"
#include "Kablunk/meta/tuple_traits.h"

#include <vector>
#include <string_view>

namespace kb::serde::json
{ // start namespace kb::serde::json

enum class json_type_t : u8
{
    // null type
    null,
    // character string value
    string,
    // unsigned 32 bit value
    u32,
    // unsigned 64 bit value
    u64,
    // signed 32 bit value
    i32,
    // signed 64 bit value
    i64,
    // float
    f32,
    // double
    f64,
    // boolean
    boolean,
    // linear array value
    vector,
    // associative map value
    map,
    // complex object value
    object,
};

// forward declaration
struct json_attribute_type;

// list of json attributes that describes the schema for serialization
struct json_schema_document
{
    // list of json attributes
    // TODO: should this be stack allocated?
    std::vector<json_attribute_type> m_attributes{};
    size_t m_size{};
};

struct json_vector_attribute_type_details
{
    // size of an individual element in bytes
    size_t m_element_size;
    // number of elements
    size_t m_element_count;
    // type of the vector element
    json_type_t m_type;
    // optional schema for the type, if it is non-trivial
    option<json_schema_document> m_schema;
};

struct json_map_attribute_value_type_details
{
    // copy of the map's key
    std::string_view m_key;
    // pointer to the map's value
    const void* m_value_ptr;
    // optional json attribute schema for the value
    option<json_schema_document> m_value_schema;
};

struct json_map_attribute_type_details
{
    using forward_iter_func_t = std::function<generator<json_map_attribute_value_type_details>()>;

    // key json type
    json_type_t m_key_type;
    // value json type
    json_type_t m_value_type;
    // size of an individual value element
    size_t m_value_element_size;
    // number of elements in the associative map
    size_t m_element_count;
    // function wrapper for a forward iterator that either returns an associative pair or empty optional
    forward_iter_func_t m_iter_func;
};

struct json_attribute_type
{
    // attribute type
    json_type_t m_type;
    // attribute name
    const char* m_name;
    // pointer to buffer to data
    const void* m_data_ptr;
    // size of data buffer in bytes
    size_t m_data_size;
    // member offset in bytes
    size_t m_offset;
    // optional json document schema for object attributes
    option<json_schema_document> m_object_attribute_schema;
    // optional details for a vector attribute's element type
    option<json_vector_attribute_type_details> m_vector_attribute_details;
    // optional details for a map attribute's element type
    option<json_map_attribute_type_details> m_map_attribute_details;
};

// template to be specialized for trivial json serializable type
template <typename T>
constexpr auto get_trivial_json_type() -> json_type_t;

// TODO: should be able to do this solely based on whether `get_trivial_json_type` is specialized...
namespace details
{ // start namespace ::details
using json_trivial_types = std::tuple<
    std::string,
    u32,
    u64,
    i32,
    i64,
    f32,
    f64,
    bool
>;
} // end namespace ::details


namespace concepts
{ // start namespace ::concepts

template <typename T>
concept JsonSerializableT = requires(const T & p_type)
{
    { p_type.get_json_schema() } -> std::same_as<json_schema_document>;
};

template <typename T>
concept JsonSerializableAndTriviallyConstructableT = JsonSerializableT<T> && std::is_trivially_constructible_v<T>;

template <typename T>
concept JsonTrivialT = std::is_same_v<
    typename meta::tuple_has_type<T, details::json_trivial_types>::type,
    std::true_type
>;


} // end namespace ::concepts

// public api for creating json attributes
auto create_json_attribute(
    std::string_view p_name,
    const auto& p_value,
    size_t p_offset
) noexcept -> json_attribute_type;

} // end namespace kb::serde::json

// template specializations for get_trivial_json_type
#include "Kablunk/serialize/kb-json/details/get_trivial_json_type_impl.h"
// template specializations for create_json_attribute
#include "Kablunk/serialize/kb-json/details/create_json_attribute_impl.h"
