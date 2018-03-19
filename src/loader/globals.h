#pragma once

#ifdef _WIN64
#define BIN_FOLDER "bin64/"
#define PLATFORM "x64"
#else
#define BIN_FOLDER "bin/"
#define PLATFORM "x86"
#endif

#define ADDONS_FOLDER BIN_FOLDER "addons/"
#define CONFIG_FOLDER "addons/loader/"
