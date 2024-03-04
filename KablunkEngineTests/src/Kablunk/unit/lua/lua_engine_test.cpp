#include "Kablunk/Core/Core.h"
#include "Kablunk/lua/lua_core.h"
#include "Kablunk/lua/lua_engine.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

TEST_CASE("lua_engine can run basic lua program", "[lua_engine_test]")
{
    kb::lua_engine lua_engine{};
    lua_engine.init();
    lua_engine.run_file(std::filesystem::path{ "lua/logger.lua" });
}
