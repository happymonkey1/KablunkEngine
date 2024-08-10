#pragma once

namespace kb::serde::json
{ // start namespace kb::serde::json

inline auto create_json_attribute(
    std::string_view p_name,
    const std::string& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::string,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(std::string),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const u32& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::u32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(u32),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const u64& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::u64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(u64),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const i32& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::i32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(i32),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const i64& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::i64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(i64),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const f32& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::f32,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(f32),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const f64& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::f64,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(f64),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

inline auto create_json_attribute(
    std::string_view p_name,
    const bool& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return {
        .m_type = json_type_t::boolean,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(bool),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}

// specialization for objects that already define a schema
template <concepts::JsonSerializableT T>
auto create_json_attribute(
    std::string_view p_name,
    const T& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return json_attribute_type{
        .m_type = json_type_t::object,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(p_value),
        .m_offset = p_offset,
        .m_object_attribute_schema = p_value.get_json_schema(),
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::nullopt,
    };
}


template <concepts::JsonSerializableT T>
auto create_json_attribute(
    std::string_view p_name,
    const std::vector<T>& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    const option<json_schema_document> element_schema = !p_value.empty() ?
        std::make_optional(p_value[0].get_json_schema()) : std::nullopt;
    return json_attribute_type{
        .m_type = json_type_t::vector,
        .m_name = p_name.data(),
        .m_data_ptr = p_value.data(),
        .m_data_size = sizeof(p_value.size() * sizeof(T)),
        .m_offset = p_offset,
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

template <concepts::JsonTrivialT T>
auto create_json_attribute(
    std::string_view p_name,
    const std::vector<T>& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    return json_attribute_type{
        .m_type = json_type_t::vector,
        .m_name = p_name.data(),
        .m_data_ptr = p_value.data(),
        .m_data_size = sizeof(p_value.size() * sizeof(T)),
        .m_offset = p_offset,
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

template <concepts::JsonSerializableT T>
auto create_json_attribute(
    std::string_view p_name,
    const unordered_flat_map<std::string, T>& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    const option<json_schema_document> value_schema = !p_value.empty() ?
        [&p_value]() -> option<json_schema_document>
        {
            // TODO: can we directly access an iterator (begin()) and retrieve one value,
            //       instead of this weird loop...
            option<json_schema_document> schema = std::nullopt;
            for (const auto& [key, value] : p_value)
            {
                schema = value.get_json_schema();
                break;
            }

            return schema;
        }() : std::nullopt;

    // TODO: I think this may be UB?
    //       https://devblogs.microsoft.com/oldnewthing/20211103-00/?p=105870
    auto iter_func = [&p_value]() noexcept -> generator<json_map_attribute_value_type_details>
        {
            for (const auto& [key, value] : p_value)
            {
                co_yield json_map_attribute_value_type_details{
                    .m_key = std::string_view{ key },
                    .m_value_ptr = static_cast<const void*>(&value),
                    .m_value_schema = value.get_json_schema(),
                };
            }

            co_return;
        };

    return json_attribute_type{
        .m_type = json_type_t::map,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(p_value),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = json_map_attribute_type_details{
            .m_key_type = json_type_t::string,
            .m_value_type = json_type_t::object,
            .m_value_element_size = sizeof(T),
            .m_element_count = p_value.size(),
            .m_iter_func = iter_func
        }
    };
}

template <concepts::JsonTrivialT T>
auto create_json_attribute(
    std::string_view p_name,
    const unordered_flat_map<std::string, T>& p_value,
    const size_t p_offset
) noexcept -> json_attribute_type
{
    // TODO: I think this may be UB?
    //       https://devblogs.microsoft.com/oldnewthing/20211103-00/?p=105870
    auto iter_func = [&p_value]() noexcept -> generator<json_map_attribute_value_type_details>
        {
            for (const auto& [key, value] : p_value)
            {
                co_yield json_map_attribute_value_type_details{
                    .m_key = std::string_view{ key },
                    .m_value_ptr = static_cast<const void*>(&value),
                    .m_value_schema = std::nullopt,
                };
            }

            co_return;
        };

    return json_attribute_type{
        .m_type = json_type_t::map,
        .m_name = p_name.data(),
        .m_data_ptr = &p_value,
        .m_data_size = sizeof(p_value),
        .m_offset = p_offset,
        .m_object_attribute_schema = std::nullopt,
        .m_vector_attribute_details = std::nullopt,
        .m_map_attribute_details = std::make_optional(json_map_attribute_type_details{
            .m_key_type = json_type_t::string,
            .m_value_type = get_trivial_json_type<T>(),
            .m_value_element_size = sizeof(T),
            .m_element_count = p_value.size(),
            .m_iter_func = iter_func
        }),
    };
}

} // end namespace kb::serde::json
