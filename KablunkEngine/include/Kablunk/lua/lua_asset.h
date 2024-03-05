#pragma once
#include "Kablunk/Asset/Asset.h"

namespace kb
{

struct lua_asset : public asset::Asset
{
    std::filesystem::path m_file_path{};
    // #NOTE: should we load file into memory?
};

}
