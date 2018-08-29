#include "file.h"
#include "encoding.h"

using namespace std;
using namespace std::filesystem;

namespace addon::utils {

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

}
