#pragma once
#include "Kablunk/serialize/kb-json/json_schema.h"


namespace kb::serde::json
{ // start namespace kb::serde::json

[[nodiscard]] auto serialize_json_schema(
    const json_schema_document& p_json_schema
) noexcept -> std::string;

// Serialize an object which already defines a json schema
template <concepts::JsonSerializable T>
[[nodiscard]] auto serialize(const T& p_data) noexcept -> std::string
{
    return serialize_json_schema(p_data.get_json_schema());
}

} // end namespace kb::serde::json
