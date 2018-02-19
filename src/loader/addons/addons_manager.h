#pragma once
#include "../windows.h"
#include <memory>
#include <vector>
#include "Addon.h"

namespace loader {
    namespace addons {

        extern std::vector<std::shared_ptr<Addon>> AddonsList;

        void RefreshAddonList();

        void InitializeAddons(UINT sdkVersion, IDirect3D9* d3d9);
        void UninitializeAddons();
        void LoadAddons(HWND hFocusWindow);
        void UnloadAddons();

    }
}
