#include "kablunkpch.h"
#include "Kablunk/serialize/kb-json/json_serialize.h"
#include "Kablunk/Core/Core.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Kablunk/Core/enum.h"

namespace kb::serde::json
{ // start namespace kb::serde::json


auto get_rapidjson_value(
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
                .m_name = nullptr,
                .m_data_ptr = element_ptr,
                .m_data_size = vec_attribute_details.m_element_size,
                .m_object_attribute_schema = vec_attribute_details.m_schema,
                // #TODO support vector and map
                .m_vector_attribute_details = std::nullopt,
                .m_map_attribute_details = std::nullopt,
            };

            json_array.PushBack(
                get_rapidjson_value(array_element_attribute_type, p_allocator),
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
                .m_name = nullptr,
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
                    .SetString(key.c_str(), static_cast<u32>(key.size()), p_allocator)
                    .Move(),
                get_rapidjson_value(value_json_attribute, p_allocator),
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
            KB_CORE_ASSERT(
                json_attribute.m_name,
                "[get_rapidjson_value]: attribute name can not be null for json_type_t::object!"
            );

            // #TODO need to mutate data pointer here...
            json_object.AddMember(
                rapidjson::StringRef(json_attribute.m_name),
                get_rapidjson_value(json_attribute, p_allocator),
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

auto serialize_json_schema(const json_schema_document& p_json_schema) noexcept -> std::string
{
    rapidjson::Document json_document{};
    json_document.SetObject();
    for (const json_attribute_type& json_attribute : p_json_schema.m_attributes)
    {
        json_document.AddMember(
            rapidjson::StringRef(json_attribute.m_name),
            get_rapidjson_value(json_attribute, json_document.GetAllocator()),
            json_document.GetAllocator()
        );
    }

    // Serialize document to string
    rapidjson::StringBuffer output_buffer;
    rapidjson::Writer writer{ output_buffer };
    json_document.Accept(writer);

    return output_buffer.GetString();
}

} // end namespace kb::serde::json
