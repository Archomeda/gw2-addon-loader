#pragma once
#include "../windows.h"
#include <memory>

namespace loader {
    namespace gui {

        class Window;
        class AddonInfoWindow;
        class SettingsWindow;

        extern std::shared_ptr<AddonInfoWindow> AddonInfoWnd;
        extern std::shared_ptr<SettingsWindow> SettingsWnd;


        void ShowWindow(Window* const window);
        void CloseWindow(Window* const window);
        bool IsWindowOpen(Window* const window);

        void Render();

    }
}
