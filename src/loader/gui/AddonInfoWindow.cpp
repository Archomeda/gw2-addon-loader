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
            // Header
            ImGui::Text(this->addon->GetName().c_str());
            ImGui::Separator();

            // Other stuff
            if (!this->addon->GetID().empty()) {
                ImGui::Text("ID: %s", this->addon->GetID().c_str());
            }
            if (!this->addon->GetVersion().empty()) {
                ImGui::Text("Version: %s", this->addon->GetVersion().c_str());
            }
            if (!this->addon->GetAuthor().empty()) {
                ImGui::Text("Author: %s", this->addon->GetAuthor().c_str());
            }
            if (!this->addon->GetFileName().empty()) {
                ImGui::Text("File name: %s", this->addon->GetFileName().c_str());
            }
            if (!this->addon->GetTypeString().empty()) {
                ImGui::Text("Type: %s", this->addon->GetTypeString().c_str());
            }
        }
    }
}
