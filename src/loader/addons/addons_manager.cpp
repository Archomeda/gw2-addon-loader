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
        
        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9) {
            GetLog()->debug("loader::addons::InitializeAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->SetSdkVersion(sdkVersion);
                (*it)->SetD3D9(d3d9);
                (*it)->Initialize();
            }
        }

        void UninitializeAddons() {
            GetLog()->debug("loader::addons::UninitializeAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->Uninitialize();
            }
        }

        void LoadAddons(HWND hFocusWindow) {
            GetLog()->debug("loader::addons::LoadAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->SetFocusWindow(hFocusWindow);
                if ((*it)->IsEnabledByConfig()) {
                    (*it)->Load();
                }
            }
        }

        void UnloadAddons() {
            GetLog()->debug("loader::addons::UnloadAddons()");
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                (*it)->Unload();
            }
        }

    }
}
