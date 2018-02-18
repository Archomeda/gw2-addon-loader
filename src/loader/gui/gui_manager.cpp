#include "gui_manager.h"
#include <map>
#include <set>
#include <imgui.h>
#include "Window.h"
#include "../utils.h"

using namespace std;

namespace loader {
    namespace gui {

        map<shared_ptr<Window>, bool> openWindows;

        void ShowWindow(const shared_ptr<Window>& window) {
            openWindows[window] = true;
        }

        void CloseWindow(const shared_ptr<Window>& window) {
            openWindows[window] = false;
        }

        bool IsWindowOpen(const shared_ptr<Window>& window) {
            return openWindows.count(window) > 0 && openWindows[window];
        }

        void Render() {
            for (auto it = openWindows.begin(); it != openWindows.end(); ) {
                if (!it->second) {
                    it = openWindows.erase(it);
                    continue;
                }
                switch (it->first->GetType()) {
                case WindowType::GenericWindow:
                    ImGui::Begin(ws2s(it->first->GetTitle()).c_str(), &it->second, it->first->GetFlags());
                    it->first->Render();
                    ImGui::End();
                    break;
                case WindowType::ModalWindow: {
                    string title = ws2s(it->first->GetTitle());
                    if (!ImGui::IsPopupOpen(title.c_str())) {
                        ImGui::OpenPopup(title.c_str());
                    }
                    if (ImGui::BeginPopupModal(title.c_str(), &it->second, it->first->GetFlags())) {
                        it->first->Render();
                        ImGui::EndPopup();
                    }
                    break;
                }
                }
                ++it;
            }
        }

    }
}
