#include "file.h"
#include "encoding.h"

using namespace std;
using namespace std::filesystem;

namespace loader::utils {

    const path GetGuildWars2Folder(const string& append) {
        wchar_t fileName[MAX_PATH];
        GetModuleFileName(NULL, fileName, sizeof(fileName));
        PathRemoveFileSpec(fileName);
        return path(fileName) / append;
    }

    const path GetSystemFolder(const string& append) {
        wchar_t systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        return (path(systemDir) / append).u8string();
    }

    bool FolderExists(const string& folderName) {
        DWORD fileType = GetFileAttributes(u16(folderName).c_str());
        if (fileType == INVALID_FILE_ATTRIBUTES) {
            return false;
        }
        else if (fileType & FILE_ATTRIBUTE_DIRECTORY) {
            return true;
        }
        return false;
    }

    bool FolderExists(const path& folderPath) {
        return FolderExists(folderPath.u8string());
    }

    bool FileExists(const string& fileName) {
        DWORD fileType = GetFileAttributes(u16(fileName).c_str());
        return fileType != INVALID_FILE_ATTRIBUTES;
    }

    bool FileExists(const path& filePath) {
        return FileExists(filePath.u8string());
    }

    FILETIME TimetToFileTime(time_t time) {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724228(v=vs.85).aspx
        FILETIME ft;
        LONGLONG ll = Int32x32To64(time, 10000000) + 116444736000000000;
        ft.dwLowDateTime = (DWORD)ll;
        ft.dwHighDateTime = ll >> 32;
        return ft;
    }

}
