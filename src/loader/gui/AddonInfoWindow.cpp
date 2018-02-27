#include "AddonInfoWindow.h"
#include <memory>
#include <imgui.h>
#include "../addons/addons_manager.h"
#include "../addons/Addon.h"

using namespace std;

namespace loader {
    namespace gui {

        AddonInfoWindow::AddonInfoWindow() {
            this->SetTitle("Addon Info");
            this->SetFlags(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        }

        void AddonInfoWindow::Render() {
            // Get selected addon info
            int selectedAddon = this->selectedAddon;
            shared_ptr<addons::Addon> addon = nullptr;
            string addonName;
            string version;
            string author;
            string fileName;
            string type;
            if (selectedAddon > -1 && selectedAddon < static_cast<int>(addons::AddonsList.size())) {
                addon = addons::AddonsList[selectedAddon];
                addonName = addon->GetName();
                addonName = !addonName.empty() ? addonName : addon->GetID();
                version = addon->GetVersion();
                author = addon->GetAuthor();
                fileName = addon->GetFileName();
                type = addon->GetAddonTypeString();
            }

            // Header
            ImGui::Text(addonName.c_str());
            ImGui::Separator();

            // Other stuff
            if (!version.empty()) {
                ImGui::Text("Version: %s", version.c_str());
            }
            if (!author.empty()) {
                ImGui::Text("Author: %s", author.c_str());
            }
            if (!fileName.empty()) {
                ImGui::Text("File name: %s", fileName.c_str());
            }
            if (!type.empty()) {
                ImGui::Text("Type: %s", type.c_str());
            }
        }
    }
}
