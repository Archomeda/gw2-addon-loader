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


        void ShowWindow(const std::shared_ptr<Window>& window);
        void CloseWindow(const std::shared_ptr<Window>& window);
        bool IsWindowOpen(const std::shared_ptr<Window>& window);

        void Render();

    }
}
