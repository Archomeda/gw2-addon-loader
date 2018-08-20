#pragma once
#include "../stdafx.h"

namespace loader::utils {

    const std::experimental::filesystem::path GetGuildWars2Folder(const std::string& append = "");
    const std::experimental::filesystem::path GetSystemFolder(const std::string& append = "");

    bool FolderExists(const std::string& folderName);
    bool FileExists(const std::string& fileName);

    FILETIME TimetToFileTime(time_t time);

}
