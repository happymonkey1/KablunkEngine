#pragma once

namespace kb::meta::refl::details
{ // start namespace kb::meta::refl::details

// reference: https://stackoverflow.com/a/68139582
template<typename T>
struct type_name
{
    constexpr static std::string_view fullname_intern()
    {
#if defined(__clang__) || defined(__GNUC__)
        return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
        return __FUNCSIG__;
#else
#   error "Unsupported compiler"
#endif
    }

    constexpr static std::string_view name()
    {
        constexpr size_t prefix_len = type_name<void>::fullname_intern().find("void");
        constexpr size_t multiple = type_name<void>::fullname_intern().size() - type_name<int>::fullname_intern().size();
        constexpr size_t dummy_len = type_name<void>::fullname_intern().size() - 4 * multiple;
        constexpr size_t target_len = (fullname_intern().size() - dummy_len) / multiple;
        constexpr std::string_view rv = fullname_intern().substr(prefix_len, target_len);
        if (rv.rfind(' ') == rv.npos)
            return rv;
        return rv.substr(rv.rfind(' ') + 1);
    }

    using type = T;
    constexpr static std::string_view value = name();
};

} // end namespace kb::meta::refl::details
