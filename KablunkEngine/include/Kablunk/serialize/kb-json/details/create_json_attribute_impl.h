#pragma once

namespace kb::serde::json
{ // start namespace kb::serde::json

inline auto create_json_attribute(
    std::string_view p_name,
    const std::string& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::string,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = p_value.size(),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const u32& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::u32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(u32),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const u64& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::u64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(u64),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const i32& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::i32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(i32),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const i64& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::i64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(i64),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const f32& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::f32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(f32),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const f64& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::f64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(f64),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const bool& p_value
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::boolean,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(bool),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

// specialization for objects that already define a schema
template <concepts::JsonSerializable T>
auto create_json_attribute(std::string_view p_name, const T& p_value) noexcept -> json_attribute_type
{
    return json_attribute_type{
        .m_type = json_type_t::object,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(p_value),
        .m_object_attribute_schema = p_value.get_json_schema(),
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}


template <concepts::JsonSerializable T>
auto create_json_attribute(std::string_view p_name, const std::vector<T>& p_value) noexcept -> json_attribute_type
{
    const option<json_schema_document> element_schema = !p_value.empty() ?
        std::make_optional(p_value[0].get_json_schema()) : std::nullopt;
    return json_attribute_type{
        .m_type = json_type_t::vector,
        .m_name = p_name.data(),
        .m_data_ptr = p_value.data(),
        .m_data_size = sizeof(p_value.size() * sizeof(T)),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::make_optional(json_vector_attribute_type_details{
            .m_element_size = sizeof(T),
            .m_element_count = p_value.size(),
            .m_type = json_type_t::object,
            .m_schema = element_schema
        }),
        .m_map_attribute_details = std::nullopt,
    };
}

// TODO: this is not working...
template <concepts::JsonTrivialT T>
auto create_json_attribute(std::string_view p_name, const std::vector<T>& p_value) noexcept -> json_attribute_type
{
    return json_attribute_type{
        .m_type = json_type_t::vector,
        .m_name = p_name.data(),
        .m_data_ptr = p_value.data(),
        .m_data_size = sizeof(p_value.size() * sizeof(T)),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::make_optional(json_vector_attribute_type_details{
            .m_element_size = sizeof(T),
            .m_element_count = p_value.size(),
            .m_type = get_trivial_json_type<T>(),
            .m_schema = std::nullopt
        }),
        .m_map_attribute_details = std::nullopt,
    };
}

#if 0
// TODO: remove
template <>
auto create_json_attribute(std::string_view p_name, const std::vector<std::string>& p_value) noexcept -> json_attribute_type
{
    const option<json_schema_document> element_schema = !p_value.empty() ?
        std::make_optional(p_value[0].get_json_schema()) : std::nullopt;
    return json_attribute_type{
        .m_type = json_type_t::vector,
        .m_name = p_name.data(),
        .m_data_ptr = p_value.data(),
        .m_data_size = sizeof(p_value.size() * sizeof(std::st)),
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::make_optional(json_vector_attribute_type_details{
            .m_element_size = sizeof(T),
            .m_element_count = p_value.size(),
            .m_type = get_trivial_json_type<T>(),
            .m_schema = std::nullopt
        }),
        .m_map_attribute_details = std::nullopt,
    };
}
#endif

} // end namespace kb::serde::json
