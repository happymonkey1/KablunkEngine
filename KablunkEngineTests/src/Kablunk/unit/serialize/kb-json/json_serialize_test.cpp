#include "Kablunk/serialize/kb-json/json_serialize.h"

#include "Kablunk/Core/Core.h"


#include <catch_amalgamated.hpp>

#include "Kablunk/serialize/kb-json/kb_json.h"
#include <string_view>

using namespace kb;

TEST_CASE("json serialization trivial", "[serde::json]")
{
    struct trivial_string
    {
        std::string m_a;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                }
            };
        }
    };

    const auto foo = trivial_string{ "hello world!" };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_a\":\"hello world!\"}" };
    REQUIRE(serialized_json_str == expected_str);
}

TEST_CASE("json serialization vector of trivial", "[serde::json]")
{
    struct vector_of_trivial
    {
        std::vector<std::string> m_a;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                }
            };
        }
    };

    const auto foo = vector_of_trivial{ { "hello", "world", "!" } };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_a\":[\"hello\",\"world\",\"!\"]}" };
    KB_CORE_INFO("[TEST]: serialized vector of object '{}'", serialized_json_str);
    REQUIRE(serialized_json_str == expected_str);
}

TEST_CASE("json serialization vector of objects", "[serde::json]")
{
    struct trivial_string
    {
        std::string m_a;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                }
            };
        }
    };

    struct foo_t
    {
        std::vector<trivial_string> m_foo;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_foo", m_foo),
                }
            };
        }
    };

    const auto foo = foo_t{ { trivial_string{ "hello world!" } } };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_foo\":[{\"m_a\":\"hello world!\"}]}" };
    KB_CORE_INFO("[TEST]: serialized vector of object '{}'", serialized_json_str);
    REQUIRE(serialized_json_str == expected_str);
}

TEST_CASE("json serialization custom object", "[serde::json]")
{
    struct trivial_string
    {
        std::string m_a;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                }
            };
        }
    };

    struct custom_object
    {
        trivial_string m_foo;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_foo", m_foo),
                }
            };
        }
    };

    const auto foo = custom_object{ "hello world!" };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_foo\":{\"m_a\":\"hello world!\"}}" };
    REQUIRE(serialized_json_str == expected_str);
    KB_CORE_INFO("[TEST]: serialized custom object '{}'", serialized_json_str);
}

TEST_CASE("json serialization custom object nested", "[serde::json]")
{
    struct trivial_string
    {
        std::string m_a;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                }
            };
        }
    };

    struct custom_object
    {
        trivial_string m_foo;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_foo", m_foo),
                }
            };
        }
    };

    struct custom_object_parent
    {
        custom_object m_bar;

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_bar", m_bar),
                }
            };
        }
    };

    const auto foo = custom_object_parent{ "hello world!" };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_bar\":{\"m_foo\":{\"m_a\":\"hello world!\"}}}" };
    REQUIRE(serialized_json_str == expected_str);
    KB_CORE_INFO("[TEST]: serialized custom object nested '{}'", serialized_json_str);
}
