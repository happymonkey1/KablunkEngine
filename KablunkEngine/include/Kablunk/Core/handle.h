#pragma once

#include <cstdint>
#include <optional>

namespace kb
{ // start namespace kb

template <typename HandleT, typename ValueT = uint32_t>
class handle
{
public:
    using value_t = ValueT;
public:
    constexpr handle() noexcept = default;
    constexpr handle(const handle&) noexcept = default;
    constexpr handle(handle&&) noexcept = default;
    constexpr ~handle() noexcept = default;

    // construct a handle from a value
    explicit constexpr handle(ValueT p_value) noexcept : m_handle{ p_value } {}

    // construct a handle from a value that can convert to the underlying type
    // #TODO concept to ensure `ConvertableT` can be converted
    template <typename ConvertableT>
    explicit constexpr handle(ConvertableT p_value) noexcept : m_handle{ p_value } {}

    auto operator=(const handle&) noexcept -> handle& = default;
    auto operator=(handle&&) noexcept -> handle& = default;

    auto operator<=>(const handle&) const = default;

    constexpr operator ValueT() const noexcept { return m_handle; }
private:
    ValueT m_handle{};
};

namespace details
{ // start namespace ::details

// empty struct used for tagging engine internal handles
struct cursor_handle_tag{};

} // end namespace ::details

using cursor_handle = handle<details::cursor_handle_tag, uint8_t>;

} // end namespace kb
