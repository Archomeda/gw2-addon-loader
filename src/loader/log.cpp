#include "log.h"
#include "globals.h"
#include "utils/file.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::utils;

namespace loader {

    map<string, shared_ptr<spdlog::logger>> loggers;
    shared_ptr<spdlog::sinks::basic_file_sink_mt> logSink;

    const string logName = "loader.log";

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
            path logFolder(fileName);
            logFolder /= LOG_FOLDER;

            create_directories(logFolder);
            if (!FolderExists(logFolder)) {
                // TODO Whenever we fail to make a log file, just return a fake logger (it would be awesome to have an ingame logger too)
                return nullptr;
            }

            path logPath(logFolder);
            logPath /= logName;

            logSink = make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.u8string(), true);
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
