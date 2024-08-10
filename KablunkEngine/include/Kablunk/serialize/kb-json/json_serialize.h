#pragma once

#include "Kablunk/serialize/kb-json/json_schema.h"
#include "Kablunk/Core/owning_buffer.h"

namespace kb::serde::json
{ // start namespace kb::serde::json

// TODO: move to separate implementation header
namespace details
{ // start namespace ::details

[[nodiscard]] auto serialize_json_schema(
    const json_schema_document& p_json_schema
) noexcept -> std::string;

[[nodiscard]] auto deserialize_json_schema(
    const std::string& p_json_string,
    const json_schema_document& p_json_schema,
    size_t p_buffer_size
) noexcept -> owning_buffer;

} // end namespace ::details


// Serialize an object which already defines a json schema
template <concepts::JsonSerializableT T>
[[nodiscard]] auto serialize(const T& p_data) noexcept -> std::string
{
    return details::serialize_json_schema(p_data.get_json_schema());
}

// Deserialize a json string into a trivially constructable object that defines a json schema
template <concepts::JsonSerializableT T>
[[nodiscard]] auto deserialize(const std::string& p_json_string) noexcept -> T
{
    T ret_value{};

    const json_schema_document schema_document = ret_value.get_json_schema();

    const size_t buffer_size = schema_document.m_size;
    KB_CORE_ASSERT(
        buffer_size == sizeof(T),
        "[serde::json::deserialize]: Buffer size {} does not equal struct size {}?",
        buffer_size,
        sizeof(T)
    );

    // Read the json string data and deserialize into a byte buffer
    auto data_buffer = details::deserialize_json_schema(p_json_string, schema_document, buffer_size);
    // Copy data into the target return value
    std::memcpy(static_cast<void*>(&ret_value), data_buffer.get(), data_buffer.size());
    return ret_value;
}

} // end namespace kb::serde::json
