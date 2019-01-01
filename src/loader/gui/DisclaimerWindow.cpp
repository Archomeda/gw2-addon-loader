#include "DisclaimerWindow.h"
#include "gui_manager.h"
#include "SettingsWindow.h"
#include "../Config.h"

using namespace std;

namespace loader::gui {

    DisclaimerWindow::DisclaimerWindow() {
        this->SetTitle("Add-on Loader Disclaimer");
        this->SetFlags(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    void DisclaimerWindow::Render() {
        // Header
        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(R"(Hello there.
This message is just here to warn you about the potential risks.

The author of this library is not associated with ArenaNet nor with any of its partners. Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners. By using this software, you agree that it is at your own risk and that you assume all responsibility. There is no warranty for using this software.

Also, the author of this library is not responsible for the potential risks by using any additional add-on in combination with this software. It's your responsibility that you use your own judgment to decide whether or not an add-on is allowed by the Guild Wars 2 Terms of Service.

Do you accept these terms? If not, you cannot use this software.)");
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        if (ImGui::Button("I accept", ImVec2(200, 0))) {
            AppConfig.SetDisclaimerAccepted(true);
            this->Close();
            ShowWindow(SettingsWnd.get());
        }
        ImGui::SameLine();
        if (ImGui::Button("I do not accept", ImVec2(200, 0))) {
            AppConfig.SetDisclaimerAccepted(false);
            this->Close();
        }
    }

}
