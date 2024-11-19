#pragma once

#include "Kablunk/Core/CoreTypes.h"

#include <rapidjson/document.h>

#include <string>
#include <string_view>

namespace kb::util
{ // start namespace kb::util

inline auto document_has_member(
    const rapidjson::Document& p_document,
    std::string_view p_key
) noexcept -> bool
{
    return p_document.HasMember(p_key.data());
}

template <typename T>
auto get_value_from_document(rapidjson::Document& p_document, std::string_view p_key) -> option<T>;


template <>
inline auto get_value_from_document(
    rapidjson::Document& p_document,
    std::string_view p_key
) -> option<std::string>
{
    if (!document_has_member(p_document, p_key))
    {
        return std::nullopt;
    }

    if (!p_document[p_key.data()].IsString())
    {
        return std::nullopt;
    }

    return p_document[p_key.data()].GetString();
}

template <>
inline auto get_value_from_document(
    rapidjson::Document& p_document,
    std::string_view p_key
) -> option<std::string_view>
{
    if (!document_has_member(p_document, p_key))
    {
        return std::nullopt;
    }

    if (!p_document[p_key.data()].IsString())
    {
        return std::nullopt;
    }

    return p_document[p_key.data()].GetString();
}

template <>
inline auto get_value_from_document(
    rapidjson::Document& p_document,
    std::string_view p_key
) -> option<u32>
{
    if (!document_has_member(p_document, p_key))
    {
        return std::nullopt;
    }

    if (!p_document[p_key.data()].IsUint())
    {
        return std::nullopt;
    }

    return p_document[p_key.data()].GetUint();
}

template <typename T>
auto document_set_value(rapidjson::Document& p_document, std::string_view p_key, T p_value) noexcept -> void;

template <>
inline auto document_set_value(
    rapidjson::Document& p_document,
    std::string_view p_key,
    std::string_view p_value
) noexcept -> void
{
    p_document.AddMember(
        rapidjson::StringRef(p_key.data()),
        rapidjson::Value(rapidjson::StringRef(p_value.data())),
        p_document.GetAllocator()
    );
}

template <>
inline auto document_set_value(
    rapidjson::Document& p_document,
    std::string_view p_key,
    u32 p_value
) noexcept -> void
{
    rapidjson::Value val{ p_value };

    p_document.AddMember(rapidjson::StringRef(p_key.data()), val, p_document.GetAllocator());
}

} // end namespace kb::util
