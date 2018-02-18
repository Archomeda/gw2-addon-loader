#pragma once
#include <spdlog/spdlog.h>

namespace loader {
    
    std::shared_ptr<spdlog::logger> GetLog();
    std::string LastErrorToString(DWORD error);

}
