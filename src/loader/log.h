#pragma once
#include "windows.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace loader {
    
    std::shared_ptr<spdlog::logger> GetLog();
    std::string LastErrorToString(DWORD error);

}
