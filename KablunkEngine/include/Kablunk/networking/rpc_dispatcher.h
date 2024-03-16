#pragma once

#include "Kablunk/Core/Core.h"

#include <optional>

#include "Kablunk/networking/rpc.h"
#include "Kablunk/meta/func_traits.h"
#include "Kablunk/meta/func_invoke.h"

namespace kb::network
{ // start namespace kb::network

namespace details
{ // start namespace ::details

} // end namespace ::details

class rpc_dispatcher final : public RefCounted
{
public:
    using response_t = std::optional<network::rpc_response>;
    using rpc_t = std::function<response_t(msgpack::object)>;

    using packet_underlying_t = std::underlying_type_t<packet_type>;
public:
    rpc_dispatcher() = default;
    ~rpc_dispatcher() noexcept override = default;

    template <typename FuncT>
    auto bind(const std::string& p_name, FuncT p_rpc_func) noexcept -> void;

    auto dispatch(const msgpack::sbuffer& p_rpc_message) const noexcept -> response_t;
private:
    template <typename FuncT>
    auto bind_to_msgpack_buffer(const std::string& p_name, FuncT p_rpc_func) noexcept -> void;
private:
    unordered_flat_map<std::string, rpc_t> m_registry{};
};

// binds a name to an rpc function
template <typename FuncT>
auto rpc_dispatcher::bind(
    const std::string& p_name,
    FuncT p_rpc_func
) noexcept -> void
{
    KB_CORE_ASSERT(
        !m_registry.contains(p_name),
        "[rpc_dispatcher]: Trying to register rpc '{}' but it is already registered?",
        p_name
    );

    bind_to_msgpack_buffer(p_name, p_rpc_func);
}

template <typename FuncT>
auto rpc_dispatcher::bind_to_msgpack_buffer(
    const std::string& p_name,
    FuncT p_rpc_func
) noexcept -> void
{
    using args_meta = ::kb::meta::func_traits<decltype(p_rpc_func)>;
    using args_type_t = typename args_meta::args_type_t;

    m_registry.emplace(
        p_name,
        [p_name, p_rpc_func](msgpack::object args)
        {
            constexpr u32 args_count = args_meta::arg_count::value;
            args_type_t args_obj{};
            args.convert(args_obj);
            ::kb::meta::invoke_func(p_rpc_func, args_obj);
            return std::make_optional(
                rpc_response{
                    .m_header = {
                        .m_id = 0x01,
                        .m_name = p_name
                    },
                    .m_data_buffer = {}
                }
            );
        }
    );
    KB_CORE_INFO("[rpc_dispatcher]: Bound rpc function '{}'", p_name);
}

// try to call an rpc function and return a response
inline auto rpc_dispatcher::dispatch(
    const msgpack::sbuffer& p_rpc_message
) const noexcept -> response_t
{
    KB_CORE_INFO("[rpc_dispatcher]: Received rpc message buffer to dispatch.");

    const auto object_handle = msgpack::unpack(
        p_rpc_message.data(),
        p_rpc_message.size()
    );
    const msgpack::object object = object_handle.get();

    rpc_request request{};
    try
    {
        object.convert(request);
    }
    catch (msgpack::type_error& err)
    {
        KB_CORE_ERROR("[rpc_dispatcher]: Could not parse buffer as an rpc call!");
        KB_CORE_ERROR("[rpc_dispatcher]:  msgpack::type_error {}", err.what());
        return std::nullopt;
    }

    // #TODO validation on packet type that it is an rpc packet
    auto&& packet_type = request.m_type;
    KB_CORE_ASSERT(
        packet_type == static_cast<packet_underlying_t>(packet_type::rpc_call),
        "[rpc_dispatcher]: rpc buffer id {} is not set to expected rpc_call={}",
        packet_type,
        static_cast<packet_underlying_t>(packet_type::rpc_call)
    );
    auto&& id = request.m_id;
    auto&& name = request.m_name;
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
