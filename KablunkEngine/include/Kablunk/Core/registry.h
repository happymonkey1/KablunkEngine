#pragma once

#include "Kablunk/Core/CoreTypes.h"

namespace kb
{ // start namespace kb

template <typename T>
class registry
{
public:
    enum class serialization_type_t : u8
    {
        yaml = 0,
        json = 1,
        bin = 2,
        none
    };

public:
    registry() = delete;
    ~registry() = delete;

    // Save the registry to disk
    auto save(
        serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_dir
    ) const noexcept -> void
    {
        derived().save(p_serialization_type, p_cache_dir);
    }

    // Load a disk cached version of the registry into memory
    [[nodiscard]] static auto load(
        serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_dir
    ) noexcept -> std::unique_ptr<registry>
    {
        return T::load(p_serialization_type, p_cache_dir);
    }

    auto derived() const noexcept -> const T& { return *static_cast<T*>(this); }
    auto derived() noexcept -> T& { return *static_cast<T*>(this); }

private:

};

} // end namespace kb
