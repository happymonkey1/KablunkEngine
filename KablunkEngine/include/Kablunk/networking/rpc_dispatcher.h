#pragma once

#include "Kablunk/Core/Core.h"

#include <optional>

#include "Kablunk/networking/rpc.h"

namespace kb::network
{ // start namespace kb::network

namespace details
{ // start namespace ::details

} // end namespace ::details

class rpc_dispatcher : public RefCounted
{
public:
    using request_t = network::rpc_request;
    using response_t = std::optional<network::rpc_response>;
    using rpc_t = std::function<response_t(msgpack::object)>;
public:
    rpc_dispatcher() = default;
    ~rpc_dispatcher() noexcept override = default;

    auto bind(const std::string& p_name, auto&& p_rpc_func) -> void;

    auto dispatch(const msgpack::sbuffer& p_rpc_message) const noexcept -> response_t;
private:
    unordered_flat_map<std::string, rpc_t> m_registry{};
};

// binds a name to an rpc function
auto rpc_dispatcher::bind(const std::string& p_name, auto&& p_rpc_func) -> void
{
    KB_CORE_ASSERT(
        !m_registry.contains(p_name),
        "[rpc_dispatcher]: Trying to register rpc '{}' but it is already registered?",
        p_name
    );

    m_registry.emplace(p_name, std::forward<decltype(p_rpc_func)>(p_rpc_func));
    KB_CORE_INFO("[rpc_dispatcher]: Bound rpc function '{}'", p_name);
}

// try to call an rpc function and return a response
inline auto rpc_dispatcher::dispatch(
    const msgpack::sbuffer& p_rpc_message
) const noexcept -> response_t
{
    const msgpack::object_handle object_handle = msgpack::unpack(
        p_rpc_message.data(),
        p_rpc_message.size()
    );
    const msgpack::object rpc_object = object_handle.get();

    request_t request{};
    rpc_object.convert(request);

    auto&& id = request.m_header.m_id;
    auto&& name = request.m_header.m_name;
    auto&& arguments = request.m_arguments;

    const auto rpc_it = m_registry.find(name);
    if (rpc_it == m_registry.end())
    {
        KB_CORE_ERROR(
            "[rpc_dispatcher]: Tried to dispatch rpc '{}', but could not find bound function call!",
            name
        );
        return std::nullopt;
    }

    KB_CORE_INFO("[rpc_dispatcher]: Dispatching call to '{}'", name);
    return rpc_it->second(arguments);
}

} // end namespace kb::network
