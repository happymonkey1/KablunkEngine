#pragma once

#include <cstdint>
#include <string_view>

namespace kb::reflect
{ // start namespace kb::reflect

/* forward declarations */
class type;

// return type info from an instance of an object
template <typename T>
inline type get_type_from_object(const T*);

class type
{
public:
    // type alias for a type id
    using type_id_t = uintptr_t;

    // copy constructor
    type(const type& p_other) noexcept;
    
    // copy assign operator
    type& operator=(const type& p_other) noexcept;

    // get the type id of this type
    type_id_t get_id() const noexcept;
    // get the human readable type name
    std::string_view get_name() const noexcept;
    // check whether the type info is actually valid
    bool is_valid() const noexcept;


public:
    /* public api */
    
    // static function to get a type object for the given template type
    static type get() noexcept;
    // static function to get a type object for a given instance of an object
    template <typename T>
    static type get(T&& p_instance) noexcept;

    /*            */
};

} // end namespace kb::reflect
