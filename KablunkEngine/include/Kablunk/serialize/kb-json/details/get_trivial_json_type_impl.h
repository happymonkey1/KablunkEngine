#pragma once

namespace kb::serde::json
{ // start namespace kb::serde::json

template <>
constexpr auto get_trivial_json_type<std::string>() -> json_type_t { return json_type_t::string; }

template <>
constexpr auto get_trivial_json_type<u32>() -> json_type_t { return json_type_t::u32; }

template <>
constexpr auto get_trivial_json_type<u64>() -> json_type_t { return json_type_t::u64; }

template <>
constexpr auto get_trivial_json_type<i32>() -> json_type_t { return json_type_t::i32; }

template <>
constexpr auto get_trivial_json_type<i64>() -> json_type_t { return json_type_t::i64; }

template <>
constexpr auto get_trivial_json_type<f32>() -> json_type_t { return json_type_t::f32; }

template <>
constexpr auto get_trivial_json_type<f64>() -> json_type_t { return json_type_t::f64; }

template <>
constexpr auto get_trivial_json_type<bool>() -> json_type_t { return json_type_t::boolean; }

} // end namespace kb::serde::json
