#include "gui_manager.h"
#include <map>
#include <imgui.h>
#include "Window.h"
#include "AddonInfoWindow.h"
#include "SettingsWindow.h"
#include "../log.h"

using namespace std;

namespace loader {
    namespace gui {

        map<Window* const, bool> openWindows;

        shared_ptr<AddonInfoWindow> AddonInfoWnd = make_shared<AddonInfoWindow>();
        shared_ptr<SettingsWindow> SettingsWnd = make_shared<SettingsWindow>();


        void ShowWindow(Window* const window) {
            GetLog()->info("Opening window {0}", window->GetTitle());
            openWindows[window] = true;
        }

        void CloseWindow(Window* const window) {
            GetLog()->info("Closing window {0}", window->GetTitle());
            openWindows[window] = false;
        }

        bool IsWindowOpen(Window* const window) {
            return openWindows.count(window) > 0 && openWindows[window];
        }


        void Render() {
            for (auto it = openWindows.begin(); it != openWindows.end(); ) {
                if (!it->second) {
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
}
