#pragma once
#include "../windows.h"
#include <memory>

namespace loader {
    namespace gui {

        class Window;
        class DisclaimerWindow;
        class SettingsWindow;

        extern std::shared_ptr<DisclaimerWindow> DisclaimerWnd;
        extern std::shared_ptr<SettingsWindow> SettingsWnd;


        void ShowWindow(Window* const window);
        void CloseWindow(Window* const window);
        bool IsWindowOpen(Window* const window);

        void Render();

    }
}
