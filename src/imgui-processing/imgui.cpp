#include "imgui.h"
#include "input.h"

using namespace std;

namespace loader::imgui {

    bool ProcessImGuiInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        ProcessInputMessage(msg, wParam, lParam);

        // Pass event to ImGui
        bool processed = gui::imgui::ProcessWndProc(msg, wParam, lParam);
        const ImGuiIO& io = ImGui::GetIO();

        // Only run these for key down/key up (incl. mouse buttons) events and when ImGui doesn't want to capture the keyboard
        if (!RepeatedPressedKeys() && !io.WantCaptureKeyboard) {
            const set<uint_fast8_t> pressedKeys = GetPressedKeys();
            const set<uint_fast8_t> settingsKeybind = AppConfig.GetSettingsKeybind();
            gui::Window* window = AppConfig.GetDisclaimerAccepted() ? static_cast<gui::Window*>(gui::SettingsWnd.get()) : static_cast<gui::Window*>(gui::DisclaimerWnd.get());

            if (pressedKeys == settingsKeybind) {
                gui::IsWindowOpen(window) ? gui::CloseWindow(window) : gui::ShowWindow(window);
                return true;
            }

#ifdef _DEBUG
            if (pressedKeys == imGuiDemoKeybind) {
                imGuiDemoOpen = !imGuiDemoOpen;
                return true;
            }
#endif
        }

        if (processed) {
            return true;
        }

        // Prevent game from receiving input if ImGui requests capture
        switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
            if (io.WantCaptureMouse) {
                return true;
            }
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            if (io.WantCaptureKeyboard) {
                return true;
            }
            break;
        case WM_CHAR:
            if (io.WantTextInput) {
                return true;
            }
            break;
        }
    }

}
