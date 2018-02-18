#include "log.h"
#include "windows.h"
#include <Shlwapi.h>
#include <filesystem>

#define LOG_FILE TEXT("addons/loader/loader.log")

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {

    shared_ptr<spdlog::logger> logger;

    shared_ptr<spdlog::logger> GetLog() {
        if (logger) {
            return logger;
        }

        TCHAR fileName[MAX_PATH];
        GetModuleFileName(NULL, fileName, sizeof(fileName));
        PathRemoveFileSpec(fileName);
        path logFileName(fileName);
        logFileName /= LOG_FILE;

        logger = spdlog::basic_logger_mt("loader", logFileName.string(), true);
#ifdef _DEBUG
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
#else
        logger->set_level(spdlog::level::info);
#endif
        return logger;
    }

    string LastErrorToString(DWORD error) {
        if (!error) {
            return "";
        }

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }

}
