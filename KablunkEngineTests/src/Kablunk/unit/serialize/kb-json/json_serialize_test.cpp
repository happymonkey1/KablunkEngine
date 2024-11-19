#include "Kablunk/serialize/kb-json/json_serialize.h"

#include "Kablunk/Core/Core.h"


#include <catch_amalgamated.hpp>

#include "Kablunk/serialize/kb-json/kb_json.h"
#include <string_view>
#include <cstddef>


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
                    serde::json::create_json_attribute("m_a", m_a, offsetof(trivial_string, m_a)),
                },
                .m_size = sizeof(trivial_string),
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
                    serde::json::create_json_attribute("m_a", m_a, offsetof(vector_of_trivial, m_a)),
                },
                .m_size = sizeof(vector_of_trivial),
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
                    serde::json::create_json_attribute("m_a", m_a, offsetof(trivial_string, m_a)),
                },
                .m_size = sizeof(trivial_string),
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
                    serde::json::create_json_attribute("m_foo", m_foo, offsetof(foo_t, m_foo)),
                },
                .m_size = sizeof(foo_t),
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
                    serde::json::create_json_attribute(
                        "m_number_map",
                        m_number_map,
                        offsetof(foo_t, m_number_map)
                    ),
                },
                .m_size = sizeof(foo_t),
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
                    serde::json::create_json_attribute("m_a", m_a, offsetof(json_object_t, m_a)),
                    serde::json::create_json_attribute("m_b", m_b, offsetof(json_object_t, m_b)),
                },
                .m_size = sizeof(json_object_t),
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
                    serde::json::create_json_attribute(
                        "m_number_map",
                        m_object_map,
                        offsetof(foo_t, m_object_map)
                    ),
                },
                .m_size = sizeof(foo_t),
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
                    serde::json::create_json_attribute("m_a", m_a, offsetof(trivial_string, m_a)),
                },
                .m_size = sizeof(trivial_string),
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
                    serde::json::create_json_attribute("m_foo", m_foo, offsetof(custom_object, m_foo)),
                },
                .m_size = sizeof(custom_object),
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
        std::string m_a{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a, offsetof(trivial_string, m_a)),
                },
                .m_size = sizeof(trivial_string),
            };
        }
    };

    struct custom_object
    {
        trivial_string m_foo{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_foo", m_foo, offsetof(custom_object, m_foo)),
                },
                .m_size = sizeof(custom_object),
            };
        }
    };

    struct custom_object_parent
    {
        custom_object m_bar{};

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute(
                        "m_bar",
                        m_bar,
                        offsetof(custom_object_parent, m_bar)
                    ),
                },
                .m_size = sizeof(custom_object_parent),
            };
        }
    };

    const auto foo = custom_object_parent{ "hello world!" };

    auto serialized_json_str = serde::json::serialize(foo);
    constexpr std::string_view expected_str{ "{\"m_bar\":{\"m_foo\":{\"m_a\":\"hello world!\"}}}" };
    REQUIRE(serialized_json_str == expected_str);
    KB_CORE_INFO("[TEST]: serialized custom object nested '{}'", serialized_json_str);
}

TEST_CASE("json deserialization trivial object", "[serde::json]")
{
    struct foo_t
    {
        std::string m_a{ "hello world" };
        u32 m_b{ 1 };
        u64 m_c{ 2 };
        i32 m_d{ -1 };
        i64 m_e{ -2 };
        f32 m_f{ 0.1f };
        f64 m_g{ 0.0029 };
        bool m_h{ false };

        auto get_json_schema() const -> serde::json::json_schema_document
        {
            return {
                .m_attributes = {
                    serde::json::create_json_attribute("m_a", m_a, offsetof(foo_t, m_a)),
                    serde::json::create_json_attribute("m_b", m_b, offsetof(foo_t, m_b)),
                    serde::json::create_json_attribute("m_c", m_c, offsetof(foo_t, m_c)),
                    serde::json::create_json_attribute("m_d", m_d, offsetof(foo_t, m_d)),
                    serde::json::create_json_attribute("m_e", m_e, offsetof(foo_t, m_e)),
                    serde::json::create_json_attribute("m_f", m_f, offsetof(foo_t, m_f)),
                    serde::json::create_json_attribute("m_g", m_g, offsetof(foo_t, m_g)),
                    serde::json::create_json_attribute("m_h", m_h, offsetof(foo_t, m_h)),
                },
                .m_size = sizeof(foo_t),
            };
        }

        auto operator<=>(const foo_t&) const = default;
    };

    const auto foo = foo_t{};

    auto serialized_json_str = serde::json::serialize(foo);
    KB_CORE_INFO("[TEST]: deeserialization of trivial using json string '{}'", serialized_json_str);

    auto deserialized_foo = serde::json::deserialize<foo_t>(serialized_json_str);
    auto deserialized_foo_str = serde::json::serialize(deserialized_foo);
    KB_CORE_INFO("[TEST]: deserialized foo '{}'", deserialized_foo_str);
    REQUIRE(foo == deserialized_foo);
}
