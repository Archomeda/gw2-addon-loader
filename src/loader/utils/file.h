#pragma once
#include "../stdafx.h"

namespace loader::utils {

    const std::filesystem::path GetGuildWars2Folder(const std::string& append = "");
    const std::filesystem::path GetSystemFolder(const std::string& append = "");

    bool FolderExists(const std::string& folderName);
    bool FolderExists(const std::filesystem::path& folderPath);
    bool FileExists(const std::string& fileName);
    bool FileExists(const std::filesystem::path& filePath);

    FILETIME TimetToFileTime(time_t time);

}
