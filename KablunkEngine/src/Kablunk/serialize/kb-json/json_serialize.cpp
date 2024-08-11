#include "kablunkpch.h"
#include "Kablunk/serialize/kb-json/json_serialize.h"
#include "Kablunk/Core/Core.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Kablunk/Core/enum.h"

namespace kb::serde::json
{ // start namespace kb::serde::json


auto serialize_value(
    const json_attribute_type& p_json_attribute,
    rapidjson::Document::AllocatorType& p_allocator
) noexcept -> rapidjson::Value
{
    auto type = p_json_attribute.m_type;
    const void* data_ptr = p_json_attribute.m_data_ptr;

    switch (type)
    {
    case json_type_t::null:
    {
        return rapidjson::Value{ rapidjson::kNullType };
    }
    case json_type_t::string:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        const auto* string_data_ptr = static_cast<const std::string*>(data_ptr);

        return rapidjson::Value{
            string_data_ptr->c_str(),
            static_cast<u32>(string_data_ptr->size())
        };
    }
    case json_type_t::u32:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const u32*>(data_ptr) };
    }
    case json_type_t::u64:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const u64*>(data_ptr) };
    }
    case json_type_t::i32:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const i32*>(data_ptr) };
    }
    case json_type_t::i64:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const i64*>(data_ptr) };
    }
    case json_type_t::f32:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const f32*>(data_ptr) };
    }
    case json_type_t::f64:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        return rapidjson::Value{ *static_cast<const f64*>(data_ptr) };
    }
    case json_type_t::boolean:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        const auto bool_value = *static_cast<const bool*>(data_ptr);
        return rapidjson::Value{ bool_value };
    }
    case json_type_t::vector:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        KB_CORE_ASSERT(
            p_json_attribute.m_vector_attribute_details.has_value(),
            "[get_rapidjson_Value]: Array object must have declared vector attribute type details!"
        );
        const auto& vec_attribute_details = p_json_attribute.m_vector_attribute_details.value();

        rapidjson::Value json_array{ rapidjson::kArrayType };

        for (size_t i = 0; i < vec_attribute_details.m_element_count; ++i)
        {
            const void* element_ptr = static_cast<const u8*>(data_ptr) + (i * vec_attribute_details.m_element_size);

            KB_CORE_ASSERT(
                vec_attribute_details.m_type != json_type_t::vector &&
                    vec_attribute_details.m_type != json_type_t::map,
                "[get_rapidjson_value]: Array object does not support vector or map element type!"
            );

            // Define a json attribute to recursively pack vector attributes
            const json_attribute_type array_element_attribute_type{
                .m_type = vec_attribute_details.m_type,
                .m_name = std::string_view{},
                .m_data_ptr = element_ptr,
                .m_data_size = vec_attribute_details.m_element_size,
                .m_object_attribute_schema = vec_attribute_details.m_schema,
                // #TODO support vector and map
                .m_vector_attribute_details = std::nullopt,
                .m_map_attribute_details = std::nullopt,
            };

            json_array.PushBack(
                serialize_value(array_element_attribute_type, p_allocator),
                p_allocator
            );
        }

        return json_array;
    }
    case json_type_t::map:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_map_attribute_details.has_value(),
            "[get_rapidjson_value]: Map object must have map attribute details defined!"
        );

        rapidjson::Value json_map{ rapidjson::kObjectType };

        const auto& map_attribute_details = p_json_attribute.m_map_attribute_details.value();

        generator<json_map_attribute_value_type_details> forward_iter_gen =
            map_attribute_details.m_iter_func();
        KB_CORE_ASSERT(
            forward_iter_gen,
            "[get_rapidjson_value]: iter_func can not be null!"
        );

        size_t index = 0;
        for (; index < map_attribute_details.m_element_count; ++index)
        {
            auto element_details = forward_iter_gen();

            const auto& key = element_details.m_key;
            const void* value_ptr = element_details.m_value_ptr;

            const auto map_value_attribute_type = map_attribute_details.m_value_type;
            KB_CORE_ASSERT(
                map_value_attribute_type != json_type_t::vector &&
                    map_value_attribute_type != json_type_t::map,
                "[get_rapidjson_value]: Map object does not support vector or map sub-type!"
            );

            json_attribute_type value_json_attribute{
                .m_type = map_attribute_details.m_value_type,
                .m_name = std::string_view{},
                .m_data_ptr = value_ptr,
                .m_data_size = map_attribute_details.m_value_element_size,
                .m_object_attribute_schema = element_details.m_value_schema,
                // TODO: support vector or map sub-type
                .m_vector_attribute_details = std::nullopt,
                .m_map_attribute_details = std::nullopt,
            };

            // TODO: can we not copy key string here?
            json_map.AddMember(
                rapidjson::Value{ rapidjson::kStringType }
                    .SetString(key.data(), static_cast<u32>(key.size()), p_allocator)
                    .Move(),
                serialize_value(value_json_attribute, p_allocator),
                p_allocator
            );
        }

        KB_CORE_ASSERT(
            index == map_attribute_details.m_element_count,
            "[get_rapidjson_value]: Expected {} elements, found {} instead?",
            map_attribute_details.m_element_count,
            index
        );

        return json_map;
    }
    case json_type_t::object:
    {
        KB_CORE_ASSERT(
            p_json_attribute.m_data_ptr,
            "[json_attribute_type]: data pointer can not be null!"
        );

        KB_CORE_ASSERT(
            p_json_attribute.m_object_attribute_schema.has_value(),
            "[get_rapidjson_value]: attribute_schema can not be null for json_type_t::object!"
        );

        rapidjson::Value json_object{ rapidjson::kObjectType };

        for (const auto& json_attribute : p_json_attribute.m_object_attribute_schema->m_attributes)
        {
            // #TODO need to mutate data pointer here...
            json_object.AddMember(
                rapidjson::StringRef(json_attribute.m_name.data()),
                serialize_value(json_attribute, p_allocator),
                p_allocator
            );
        }

        return json_object;
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[json_attribute_type]: Unhandled json_type_t {}!",
            to_underlying(type)
        );
        return rapidjson::Value{ rapidjson::kNullType };
    }
    }
}

auto deserialize_value(
    const json_attribute_type& p_json_attribute,
    rapidjson::Value p_rapidjson_value,
    u8* p_dest_ptr
) -> size_t
{
    const auto type = p_json_attribute.m_type;

    switch (type)
    {
    case json_type_t::string:
    {
        new (p_dest_ptr) std::string{ p_rapidjson_value.GetString() };
        return sizeof(std::string);
    }
    case json_type_t::u32:
    {
        new (p_dest_ptr) u32{ p_rapidjson_value.GetUint() };
        return sizeof(u32);
    }
    case json_type_t::u64:
    {
        new (p_dest_ptr) u64{ p_rapidjson_value.GetUint64() };
        return sizeof(u64);
    }
    case json_type_t::i32:
    {
        new (p_dest_ptr) i32{ p_rapidjson_value.GetInt() };
        return sizeof(i32);
    }
    case json_type_t::i64:
    {
        new (p_dest_ptr) i64{ p_rapidjson_value.GetInt64() };
        return sizeof(i64);
    }
    case json_type_t::f32:
    {
        new (p_dest_ptr) f32{ p_rapidjson_value.GetFloat() };
        return sizeof(f32);
    }
    case json_type_t::f64:
    {
        new (p_dest_ptr) f64{ p_rapidjson_value.GetDouble() };
        return sizeof(f64);
    }
    case json_type_t::boolean:
    {
        new (p_dest_ptr) bool{ p_rapidjson_value.GetBool() };
        return sizeof(bool);
    }
    case json_type_t::vector:
    {
        KB_CORE_ASSERT(false, "not implemented!");
        return 0;
    }
    case json_type_t::map:
    {
        KB_CORE_ASSERT(false, "not implemented!");
        return 0;
    }
    case json_type_t::object:
    {
        const auto json_object = p_rapidjson_value.GetObj();

        KB_CORE_ASSERT(
            p_json_attribute.m_object_attribute_schema.has_value(),
            "[deserialize_value]: Json object must have a defined schema!"
        );
        const auto& object_json_schema = p_json_attribute.m_object_attribute_schema.value();

        owning_buffer json_object_data_buffer{ object_json_schema.m_size };
        const auto head_object_ptr = static_cast<u8*>(json_object_data_buffer.get());
        size_t cur_size = 0ull;

        size_t index = 0ull;
        for (auto&& [rapidjson_value, allocator] : json_object)
        {
            KB_CORE_ASSERT(
                index < object_json_schema.m_attributes.size(),
                "[deserialize_value]: Index out of bounds of sub-object json attributes!"
            );
            const auto& json_attribute = object_json_schema.m_attributes[index++];

            auto* member_ptr = head_object_ptr + json_attribute.m_offset;

            KB_CORE_ASSERT(
                cur_size + json_attribute.m_data_size <= object_json_schema.m_size,
                "[deserialize_value]: Buffer overflow while trying to emplace sub-object member into data buffer!"
            );

            const auto emplaced_size = deserialize_value(
                json_attribute,
                std::move(rapidjson_value),
                member_ptr
            );

            cur_size += emplaced_size;
        }

        KB_CORE_ASSERT(
            cur_size == object_json_schema.m_size,
            "[deserialize_value]: Emplaced size {} does not match object size {}?",
            cur_size,
            object_json_schema.m_size
        );

        std::memcpy(p_dest_ptr, head_object_ptr, object_json_schema.m_size);

        return object_json_schema.m_size;
    }
    case json_type_t::null:
    {
        KB_CORE_ASSERT(false, "[deserialize_value]: Trying to deserialize a null value?");
        return 0;
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[deserialize_value]: Unhandled json_type_t '{}'",
            to_underlying(type)
        );
        return 0;
    }
    }
}

auto details::serialize_json_schema(const json_schema_document& p_json_schema) noexcept -> std::string
{
    rapidjson::Document json_document{};
    json_document.SetObject();

    // Iterate attributes, translating to a corresponding rapidjson value
    // then add to the document
    for (const json_attribute_type& json_attribute : p_json_schema.m_attributes)
    {
        KB_CORE_ASSERT(
            !json_attribute.m_name.empty(),
            "[serialize_value]: Attribute name can not be empty when serializing member!"
        );

        json_document.AddMember(
            rapidjson::StringRef(json_attribute.m_name.data()),
            serialize_value(json_attribute, json_document.GetAllocator()),
            json_document.GetAllocator()
        );
    }

    // Serialize document to string
    rapidjson::StringBuffer output_buffer;
    rapidjson::Writer writer{ output_buffer };
    json_document.Accept(writer);

    return output_buffer.GetString();
}

auto details::deserialize_json_schema(
    const std::string& p_json_string,
    const json_schema_document& p_json_schema,
    const size_t p_buffer_size
) noexcept -> owning_buffer
{
    rapidjson::Document json_document{};
    json_document.Parse(p_json_string.c_str());

    KB_CORE_ASSERT(
        json_document.IsObject(),
        "[deserialize_json_schema]: Failed to deserialize top level json object!"
    );

    // allocate a buffer for all attributes
    owning_buffer data_buffer{ p_buffer_size };
    u8* buffer_head_ptr = static_cast<u8*>(data_buffer.get());
    size_t cur_size = 0;

    // iterate each of the document values (and json_attributes) and emplace them into the buffer
    size_t index = 0;
    for (auto&& rapidjson_value : json_document.GetObj())
    {
        KB_CORE_ASSERT(
            index < p_json_schema.m_attributes.size(),
            "[deserialize_json_schema]: Trying to index out of attribute bounds!"
        );
        const auto& json_attribute = p_json_schema.m_attributes[index++];
        auto&& name = std::string_view{ rapidjson_value.name.GetString() };

#ifdef KB_DEBUG
        {
            std::string_view attribute_name{ json_attribute.m_name };
            KB_CORE_ASSERT(
                name == attribute_name,
                "[deserialize_json_schema]: Expected attribute with name {}, found {} instead?",
                attribute_name,
                name
            );
        }
#endif

        KB_CORE_ASSERT(
            cur_size + json_attribute.m_data_size <= p_buffer_size,
            "[deserialize_json_schema]: Data buffer out of bounds?"
        );

        u8* member_ptr = buffer_head_ptr + json_attribute.m_offset;

        // Value is directly deserialized into the data buffer
        const auto emplaced_size = deserialize_value(
            json_attribute,
            std::move(rapidjson_value.value),
            member_ptr
        );
        KB_CORE_ASSERT(
            emplaced_size == json_attribute.m_data_size,
            "[deserialize_json_schema]: Expected to emplace value with size {}, but is {} instead?",
            json_attribute.m_data_size,
            emplaced_size
        );

        cur_size += json_attribute.m_data_size;
    }

    return data_buffer;
}

} // end namespace kb::serde::json
