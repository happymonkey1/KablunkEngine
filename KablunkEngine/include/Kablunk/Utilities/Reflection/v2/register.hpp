#pragma once

#include <string_view>

namespace kb::reflect
{ // start namespace kb::reflect

class registration
{
public:

    /* registration for structs */
    template <typename struct_t>
    class struct_
    {
    public:
        // default constructor
        struct_() = default;
        // overloaded constructor to begin registering a named struct
        struct_(std::string_view p_struct_name);
        // destructor
        ~struct_() = default;


        // register a constructor for the struct
        void ctor();

        // register a property for the struct
        void property();

        // register a method for the struct
        void method();

    private:
        // copy constructor
        struct_(const struct_& p_other);
        // copy assign operator
        struct_& operator=(const struct_& p_other);

    private:

    };
    /*                          */

};

} // end namespace kb::reflect
