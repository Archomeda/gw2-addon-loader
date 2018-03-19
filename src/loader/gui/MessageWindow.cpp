#include "MessageWindow.h"

using namespace std;

namespace loader {
    namespace gui {

        MessageWindow::MessageWindow(const string& title, const string& message) : Message(message) {
            this->SetTitle(title);
            this->SetFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
            this->SetType(WindowType::ModalWindow);
        }

        void MessageWindow::Render() {
            ImGui::BeginChild("##MessagePanel", ImVec2(480, 80));
            {
                ImGui::TextWrapped(this->Message.c_str());
            }
            ImGui::EndChild();
            ImGui::BeginChild("##Buttons", ImVec2(480, ImGui::GetFrameHeightWithSpacing()));
            {
                ImGui::SameLine((480 - 120) / 2);
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    this->Close();
                }
            }
            ImGui::EndChild();
        }

        unique_ptr<MessageWindow> MessageWindow::ShowMessageWindow(const string& title, const string& message) {
            unique_ptr<MessageWindow> window = make_unique<MessageWindow>(title, message);
            window->Show();
            return window;
        }

    }
}
