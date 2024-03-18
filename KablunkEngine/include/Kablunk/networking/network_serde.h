#pragma once

#include <Kablunk/Core/CoreTypes.h>
#include <msgpack.hpp>
#include <glm/glm.hpp>

// Non-intrusive msgpack adaptors (ex: vendor types)

namespace kb::network
{ // start namespace kb::network
} // end namespace kb::network


namespace msgpack
{ // start namespace msgpack
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{ // start api namespace
namespace adaptor
{ // start namespace ::adapator

// convert msgpack::object -> glm::vec3
template <>
struct convert<glm::vec3>
{
    msgpack::object const& operator()(const msgpack::object& p_object, glm::vec3& p_value) const
    {
        if (p_object.type != msgpack::type::ARRAY) throw msgpack::type_error();
        if (p_object.via.array.size != 3) throw msgpack::type_error();

        p_object.via.array.ptr[0] >> p_value.x;
        p_object.via.array.ptr[1] >> p_value.y;
        p_object.via.array.ptr[2] >> p_value.z;

        return p_object;
    }
};

// pack glm::vec3
template<>
struct pack<glm::vec3>
{
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& p_packer, const glm::vec3& p_value) const
    {
        // packing member variables as an array.
        p_packer.pack_array(3);
        p_packer.pack(p_value.x);
        p_packer.pack(p_value.y);
        p_packer.pack(p_value.z);
        return p_packer;
    }
};

// glm::vec3 -> object with zone alloc
template <>
struct object_with_zone<glm::vec3>
{
    void operator()(msgpack::object::with_zone& p_zoned_object, const glm::vec3& p_value) const
    {
        p_zoned_object.type = type::ARRAY;
        p_zoned_object.via.array.size = 3;
        p_zoned_object.via.array.ptr = static_cast<msgpack::object*>(
            p_zoned_object.zone.allocate_align(sizeof(msgpack::object) * p_zoned_object.via.array.size)
        );
        p_zoned_object.via.array.ptr[0] = msgpack::object(p_value.x, p_zoned_object.zone);
        p_zoned_object.via.array.ptr[1] = msgpack::object(p_value.y, p_zoned_object.zone);
        p_zoned_object.via.array.ptr[2] = msgpack::object(p_value.z, p_zoned_object.zone);
    }
};

// convert msgpack::object -> glm::vec2
template <>
struct convert<glm::vec2>
{
    msgpack::object const& operator()(const msgpack::object& p_object, glm::vec2& p_value) const
    {
        if (p_object.type != msgpack::type::ARRAY) throw msgpack::type_error();
        if (p_object.via.array.size != 2) throw msgpack::type_error();

        p_object.via.array.ptr[0] >> p_value.x;
        p_object.via.array.ptr[1] >> p_value.y;

        return p_object;
    }
};

// pack glm::vec2
template<>
struct pack<glm::vec2>
{
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& p_packer, const glm::vec2& p_value) const
    {
        // packing member variables as an array.
        p_packer.pack_array(2);
        p_packer.pack(p_value.x);
        p_packer.pack(p_value.y);
        return p_packer;
    }
};

// glm::vec2 -> object with zone alloc
template <>
struct object_with_zone<glm::vec2>
{
    void operator()(msgpack::object::with_zone& p_zoned_object, const glm::vec2& p_value) const
    {
        p_zoned_object.type = type::ARRAY;
        p_zoned_object.via.array.size = 2;
        p_zoned_object.via.array.ptr = static_cast<msgpack::object*>(
            p_zoned_object.zone.allocate_align(sizeof(msgpack::object) * p_zoned_object.via.array.size)
            );
        p_zoned_object.via.array.ptr[0] = msgpack::object(p_value.x, p_zoned_object.zone);
        p_zoned_object.via.array.ptr[1] = msgpack::object(p_value.y, p_zoned_object.zone);
    }
};

} // end namespace ::adaptor
} // end api namspace
} // end namsepace msgpack
