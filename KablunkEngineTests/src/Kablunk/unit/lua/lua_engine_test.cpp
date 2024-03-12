#include "Kablunk/Core/Core.h"
#include "Kablunk/lua/lua_core.h"
#include "Kablunk/lua/lua_engine.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

using namespace kb;

TEST_CASE("lua_engine can run basic lua program", "[lua_engine_test]")
{
    // test setup
    auto asset_manager = ref<asset::AssetManager>::Create();


    kb::lua_engine lua_engine{ asset_manager };
    lua_engine.init();
    lua_engine.run_file(std::filesystem::path{ "KablunkEngineTests/resources/lua/logger.lua" });
}
