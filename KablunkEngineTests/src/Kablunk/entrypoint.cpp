#include "Kablunk/Core/Core.h"
#include <catch_amalgamated.hpp>

int main(int argc, char* argv[])
{
    ::kb::Singleton<kb::Logger>::get().init();

    int result = Catch::Session().run(argc, argv);

    // your clean-up...

    return result;
}
