#pragma once
#include "../windows.h"
#include <filesystem>
#include <time.h>
#include <string>

namespace loader {
    namespace utils {

        const std::experimental::filesystem::v1::path GetGuildWars2Folder(const std::string& append = "");

        bool FolderExists(const std::string& folderName);
        bool FileExists(const std::string& fileName);

        FILETIME TimetToFileTime(time_t time);

    }
}
