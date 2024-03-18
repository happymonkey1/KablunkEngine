#pragma once

#include "Kablunk/Core/Core.h"

#include <optional>

#include "Kablunk/networking/rpc.h"
#include "Kablunk/meta/func_traits.h"
#include "Kablunk/meta/func_invoke.h"
#include "Kablunk/networking/client_info.h"
#include "Kablunk/networking/network_utils.h"

namespace kb::network
{ // start namespace kb::network

namespace details
{ // start namespace ::details

} // end namespace ::details

class rpc_dispatcher
{
public:
    using response_t = std::optional<network::rpc_response>;
    using rpc_t = std::function<response_t(const client_info&, u32, const msgpack::object&)>;
    // underlying integral type of `packet_type`
    using packet_underlying_t = std::underlying_type_t<packet_type>;

public:
    rpc_dispatcher() = default;
    ~rpc_dispatcher() noexcept = default;

    rpc_dispatcher(const rpc_dispatcher&) noexcept = delete;
    rpc_dispatcher(rpc_dispatcher&&) noexcept = delete;

    // bind a function to an rpc wrapper and store
    template <typename FuncT>
    auto bind(const std::string& p_name, FuncT p_rpc_func) noexcept -> void;

    // invoke rpc based on buffer data
    auto dispatch(
        const client_info& p_client_info,
        const msgpack::object& p_rpc_message
    ) const noexcept -> response_t;

    auto operator=(const rpc_dispatcher&) noexcept -> rpc_dispatcher& = delete;
    auto operator=(const rpc_dispatcher&&) noexcept -> rpc_dispatcher& = delete;
private:
    template <typename FuncT>
    auto bind_to_msgpack_buffer(const std::string& p_name, FuncT p_rpc_func) noexcept -> void;
private:
    // map of rpc name to rpc wrapper
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
    // magic to get arguments of the function
    using args_meta = ::kb::meta::func_traits<decltype(p_rpc_func)>;
    using args_type_t = typename args_meta::args_type_t;

    // bind the function name to an rpc wrapper
    m_registry.emplace(
        p_name,
        [p_name, p_rpc_func](
            [[maybe_unused]] const client_info& p_client_info,
            const u32 p_packet_id,
            const msgpack::object& args)
        {
            constexpr u32 args_count = args_meta::arg_count::value;
            args_type_t args_obj{};
            // #TODO validate argument count is correct
            args.convert(args_obj);
            ::kb::meta::invoke_func(p_rpc_func, args_obj);
            return std::make_optional(
                rpc_response{
                    .m_type = static_cast<packet_underlying_t>(packet_type::kb_rpc_response),
                    // #TODO fix
                    .m_id = p_packet_id,
                    .m_name = p_name,
                    .m_data_buffer = {},
                }
            );
        }
    );
    KB_CORE_INFO("[rpc_dispatcher]: Bound rpc function '{}'", p_name);
}

// try to call an rpc function and return a response
inline auto rpc_dispatcher::dispatch(
    const client_info& p_client_info,
    const msgpack::object& p_rpc_message
) const noexcept -> response_t
{
    KB_CORE_INFO("[rpc_dispatcher]: Received rpc message buffer to dispatch.");

    if (!p_client_info.m_authenticated)
    {
        KB_CORE_WARN("[rpc_dispatcher]: client '{}' is not authenticated!", p_client_info.m_client_id);
        return std::nullopt;
    }

    const auto request_result = util::convert_object<rpc_request>(p_rpc_message);
    if (!request_result)
        return std::nullopt;

    const auto request = std::move(request_result.value());

    auto&& packet_type = request.m_type;
    KB_CORE_ASSERT(
        packet_type == static_cast<underlying_packet_type_t>(packet_type::kb_rpc_call),
        "[network_server]: rpc call packet type is not '{}'?",
        static_cast<underlying_packet_type_t>(packet_type::kb_rpc_call)
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
    return rpc_it->second(p_client_info, id, arguments);
}

} // end namespace kb::network
