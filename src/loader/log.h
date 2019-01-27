#pragma once
#include "stdafx.h"

#define ADDONS_LOG() loader::GetLog("addons")
#define CONFIG_LOG() loader::GetLog("config")
#define DIAG_LOG() loader::GetLog("diag")
#define GUI_LOG() loader::GetLog("gui")
#define HOOKS_LOG() loader::GetLog("hooks")
#define LOADER_LOG() loader::GetLog()
#define MUMBLE_LOG() loader::GetLog("mumble")
#define UPDATERS_LOG() loader::GetLog("updaters")

namespace loader {

    const std::shared_ptr<spdlog::logger> GetLog();
    const std::shared_ptr<spdlog::logger> GetLog(const std::string& name);
    const std::string LastErrorToString();

}
