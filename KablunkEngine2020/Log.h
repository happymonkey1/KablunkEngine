#pragma once
#include "Core.h"

#include <loguru.hpp>
#include <loguru.cpp>



#define KABLUNK_CORE_ERROR(...) RAW_LOG_F(ERROR, "KablunkEngine | %s", __VA_ARGS__);
#define KABLUNK_CORE_WARN(...) RAW_LOG_F(WARNING, "KablunkEngine | %s", __VA_ARGS__);
#define KABLUNK_CORE_INFO(...) RAW_LOG_F(INFO, "KablunkEngine | %s", __VA_ARGS__);

#define CLIENT_CORE_ERROR(...) RAW_LOG_F(ERROR, "KablunkEngine | %s", __VA_ARGS__);
#define CLIENT_CORE_WARN(...) RAW_LOG_F(WARNING, "KablunkEngine | %s", __VA_ARGS__);
#define CLIENT_CORE_INFO(...) RAW_LOG_F(INFO, "KablunkEngine | %s", __VA_ARGS__);

