#include "addons_manager.h"
#include <filesystem>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "../Config.h"
#include "../log.h"

#ifdef _WIN64
#define ADDONS_FOLDER "bin64/addons/"
#else
#define ADDONS_FOLDER "bin/addons/"
#endif

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace addons {

        vector<shared_ptr<Addon>> AddonsList;

        void RefreshAddonList() {
            GetLog()->debug("loader::addons::RefreshAddonList()");

            // Clear our list
            //TODO: Don't clear this, but instead check what's removed and what's new, because we do lose some references here otherwise
            // Until ^ isn't solved, refreshing can only be done by restarting
            AddonsList.clear();

            // Create path
            TCHAR fileName[MAX_PATH];
            GetModuleFileName(NULL, fileName, sizeof(fileName));
            PathRemoveFileSpec(fileName);
            path addonsFolder(fileName);
            addonsFolder /= ADDONS_FOLDER;
            GetLog()->info("Using addon folder {0}", addonsFolder.string());
            if (!PathFileExists(addonsFolder.c_str())) {
                SHCreateDirectoryEx(NULL, addonsFolder.c_str(), NULL);
            }

            // Iterate and find DLL files
            for (auto& pathFile : directory_iterator(addonsFolder)) {
                if (pathFile.path().extension() == ".dll") {
                    GetLog()->info("Found {0}", pathFile.path().string());
                    AddonsList.push_back(make_shared<Addon>(pathFile.path().wstring()));
                }
            }

            sort(AddonsList.begin(), AddonsList.end(), [](shared_ptr<Addon> a, shared_ptr<Addon> b) {
                return AppConfig.GetAddonOrder(b->GetFileName()) > AppConfig.GetAddonOrder(a->GetFileName());
            });
        }
    }
}
