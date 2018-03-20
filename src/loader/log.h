#pragma once
#include "windows.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace loader {

    const std::shared_ptr<spdlog::logger> GetLog();
    const std::string LastErrorToString(DWORD error);

}
