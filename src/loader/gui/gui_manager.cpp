#include "gui_manager.h"
#include "Window.h"
#include "DisclaimerWindow.h"
#include "SettingsWindow.h"
#include "../log.h"

using namespace std;

namespace loader::gui {

    map<Window* const, bool> openWindows;

    shared_ptr<DisclaimerWindow> DisclaimerWnd = make_shared<DisclaimerWindow>();
    shared_ptr<SettingsWindow> SettingsWnd = make_shared<SettingsWindow>();


    void ShowWindow(Window* const window) {
        GUI_LOG()->debug("Opening window {0}", window->GetTitle());
        openWindows[window] = true;
        window->OnShow();
    }

    void CloseWindow(Window* const window) {
        openWindows[window] = false;
    }

    void OnCloseWindow(Window* const window) {
        GUI_LOG()->debug("Closing window {0}", window->GetTitle());
        window->OnClose();
    }

    bool IsWindowOpen(Window* const window) {
        return openWindows.count(window) > 0 && openWindows[window];
    }


    void Render() {
        for (auto it = openWindows.begin(); it != openWindows.end(); ) {
            if (!it->second) {
                // Remove the closed window
                OnCloseWindow(it->first);
                it = openWindows.erase(it);
                continue;
            }
            auto wnd = it->first;
            wnd->BeginStyle();
            switch (wnd->GetType()) {
            case WindowType::GenericWindow:
                ImGui::Begin(wnd->GetTitle().c_str(), &it->second, wnd->GetFlags());
                wnd->Render();
                ImGui::End();
                break;
            case WindowType::ModalWindow: {
                string title = wnd->GetTitle();
                if (!ImGui::IsPopupOpen(title.c_str())) {
                    ImGui::OpenPopup(title.c_str());
                }
                if (ImGui::BeginPopupModal(title.c_str(), &it->second, wnd->GetFlags())) {
                    wnd->Render();
                    ImGui::EndPopup();
                }
                break;
            }
            }
            wnd->EndStyle();
            ++it;
        }
    }

}
