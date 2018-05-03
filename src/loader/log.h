#pragma once
#include "windows.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

#define ADDONS_LOG() loader::GetLog("addons")
#define CONFIG_LOG() loader::GetLog("config")
#define GUI_LOG() loader::GetLog("gui")
#define HOOKS_LOG() loader::GetLog("hooks")
#define LOADER_LOG() loader::GetLog()
#define MUMBLE_LOG() loader::GetLog("mumble")
#define UPDATERS_LOG() loader::GetLog("updaters")

namespace loader {

    const std::shared_ptr<spdlog::logger> GetLog();
    const std::shared_ptr<spdlog::logger> GetLog(const std::string& name);
    const std::string LastErrorToString(DWORD error);

}
