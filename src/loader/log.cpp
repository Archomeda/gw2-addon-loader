#include "log.h"
#include <Shlwapi.h>
#include <filesystem>
#include <map>

#define LOG_FILE "addons/loader/loader.log"

using namespace std;
using namespace std::experimental::filesystem;

namespace loader {

    map<string, shared_ptr<spdlog::logger>> loggers;
    shared_ptr<spdlog::sinks::simple_file_sink_mt> logSink;

    const shared_ptr<spdlog::logger> GetLog() {
        return GetLog("loader");
    }

    const shared_ptr<spdlog::logger> GetLog(const string& name) {
        if (loggers.find(name) != loggers.end()) {
            return loggers[name];
        }

        if (logSink == nullptr) {
            TCHAR fileName[MAX_PATH];
            GetModuleFileName(NULL, fileName, sizeof(fileName));
            PathRemoveFileSpec(fileName);
            path logFileName(fileName);
            logFileName /= LOG_FILE;

            logSink = make_shared<spdlog::sinks::simple_file_sink_mt>(logFileName.u8string(), true);
        }

        shared_ptr<spdlog::logger> logger = make_shared<spdlog::logger>(name, logSink);
#ifdef _DEBUG
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
#else
        logger->set_level(spdlog::level::info);
#endif
        return logger;
    }

    const string LastErrorToString(DWORD error) {
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
