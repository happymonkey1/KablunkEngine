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

TEST_CASE("json serialization map of trivial", "[serde::json]")
{
    struct foo_t
    {
        unordered_flat_map<std::string, u32> m_number_map{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_number_map", m_number_map),
                }
            };
        }
    };

    foo_t foo{
        .m_number_map = {
            { "1", 1 },
            { "2", 2 },
            { "3", 3 },
            { "100", 100 }
        }
    };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_number_map\":{\"3\":3,\"2\":2,\"1\":1,\"100\":100}}" };
    KB_CORE_INFO("[TEST]: serialized map of trivial '{}'", serialized_json_str);
    REQUIRE(serialized_json_str == expected_str);
}


TEST_CASE("json serialization map of object", "[serde::json]")
{
    struct json_object_t
    {
        u32 m_a{};
        std::string m_b{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a),
                    serde::json::create_json_attribute("m_b", m_b),
                }
            };
        }
    };

    struct foo_t
    {
        unordered_flat_map<std::string, json_object_t> m_object_map{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_number_map", m_object_map),
                }
            };
        }
    };

    foo_t foo{
        .m_object_map = {
            { "1", json_object_t{ 1, "hello" } },
            { "2", json_object_t{ 2, "world" } },
            { "3", json_object_t{ 3, "!" } },
        }
    };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_number_map\":{\"3\":{\"m_a\":3,\"m_b\":\"!\"},\"2\":{\"m_a\":2,\"m_b\":\"world\"},\"1\":{\"m_a\":1,\"m_b\":\"hello\"}}}" };
    KB_CORE_INFO("[TEST]: serialized map of object '{}'", serialized_json_str);
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
