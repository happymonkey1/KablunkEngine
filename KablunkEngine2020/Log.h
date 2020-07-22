#pragma once
#include "Core.h"

#include <loguru.hpp>



#define LOGURU_THREADNAME_WIDTH 0


#define KABLUNK_CORE_ERROR(...) RAW_LOG_F(ERROR, "Kablunk| %s", __VA_ARGS__);
#define KABLUNK_CORE_WARN(...) RAW_LOG_F(WARNING, "Kablunk| %s", __VA_ARGS__);
#define KABLUNK_CORE_INFO(...) RAW_LOG_F(INFO, __VA_ARGS__);

#define KABLUNK_CLIENT_ERROR(...) RAW_LOG_F(ERROR, "APP| %s", __VA_ARGS__);
#define KABLUNK_CLIENT_WARN(...) RAW_LOG_F(WARNING, "APP| %s", __VA_ARGS__);
#define KABLUNK_CLIENT_INFO(...) RAW_LOG_F(INFO, "APP| %s", __VA_ARGS__);
