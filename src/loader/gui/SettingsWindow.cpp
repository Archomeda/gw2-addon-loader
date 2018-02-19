#include "SettingsWindow.h"
#include "../windows.h"
#include <shellapi.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <imgui.h>
#include "MessageWindow.h"
#include "../addons/addons_manager.h"
#include "../addons/Addon.h"
#include "../Config.h"
#include "../utils.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

namespace loader {
    namespace gui {

        SettingsWindow::SettingsWindow() {
            this->SetTitle(L"Addons");
            this->SetFlags(ImGuiWindowFlags_NoCollapse);
        }

        void SettingsWindow::Render() {
            // Filter the list first
            vector<shared_ptr<addons::Addon>> addons;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                if ((*it)->GetAddonType() != addons::AddonType::UnknownAddon) {
                    addons.push_back(*it);
                }
            }

            // Render window
            ImGui::BeginChild("##LeftPanel", ImVec2(150, -1), false);
            {
                vector<string> names;
                for (auto& addon : addons) {
                    names.push_back(ws2s(addon->GetID()));
                }
                vector<char*> cnames;
                cnames.reserve(names.size());
                for (size_t i = 0; i < names.size(); ++i) {
                    cnames.push_back(const_cast<char*>(names[i].c_str()));
                }
                if (addons.size() > 0) {
                    ImGui::PushItemWidth(-1);
                    ImGui::ListBox("##Addons", &this->selectedAddon, &cnames[0], static_cast<int>(addons.size()));
                    ImGui::PopItemWidth();
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                shared_ptr<addons::Addon> addon = nullptr;
                wstring filePath;
                wstring fileName;
                addons::types::AddonState state;
                string stateString;
                string file;
                string type;
                string id;
                string productName;
                string author;
                string description;
                string version;
                wstring homepage;
                if (this->selectedAddon > -1 && this->selectedAddon < static_cast<int>(addons.size())) {
                    addon = addons[this->selectedAddon];
                    filePath = addon->GetFilePath();
                    fileName = addon->GetFileName();
                    state = addon->GetTypeImpl()->GetAddonState();
                    stateString = ws2s(addon->GetTypeImpl()->GetAddonStateString());
                    file = ws2s(fileName);
                    type = ws2s(addon->GetAddonTypeString());
                    id = ws2s(addon->GetID());
                    productName = ws2s(addon->GetProductName());
                    author = ws2s(addon->GetAuthor());
                    description = ws2s(addon->GetDescription());
                    version = ws2s(addon->GetVersion());
                    homepage = addon->GetHomepage();
                }
                ImGui::BeginChild("##Addon", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us
                {
                    ImGui::BeginGroup();
                    {
                        if (!productName.empty()) {
                            ImGui::Text(productName.c_str());
                        }
                        else {
                            ImGui::Text(id.c_str());
                        }
                        ImGui::SameLine(ImGui::GetWindowWidth() - 75);
                        if (!homepage.empty()) {
                            if (ImGui::Button("Homepage", ImVec2(75, 0))) {
                                ShellExecute(0, 0, homepage.c_str(), 0, 0, SW_SHOW);
                            }
                        }
                    }
                    ImGui::EndGroup();
                    ImGui::Separator();
                    if (!file.empty()) {
                        ImGui::Text("File: %s", file.c_str());
                    }
                    if (!author.empty()) {
                        ImGui::Text("Author: %s", author.c_str());
                    }
                    if (!version.empty()) {
                        ImGui::Text("Version: %s", version.c_str());
                    }
                    if (!type.empty()) {
                        ImGui::Text("Type: %s", type.c_str());
                    }
                    if (!id.empty()) {
                        ImGui::Text("State:");
                        ImGui::SameLine();
                        ImColor color(120, 120, 120);
                        switch (state) {
                        case addons::types::AddonState::LoadingState:
                            color = ImColor(255, 255, 0);
                            break;
                        case addons::types::AddonState::ActivatedOnRestartState:
                        case addons::types::AddonState::DeactivatedOnRestartState:
                            color = ImColor(0, 255, 255);
                            break;
                        case addons::types::AddonState::LoadedState:
                            color = ImColor(0, 255, 0);
                            break;
                        case addons::types::AddonState::ErroredState:
                            color = ImColor(255, 0, 0);
                            break;
                        }
                        ImGui::TextColored(color, stateString.c_str());
                    }
                    if (!description.empty()) {
                        ImGui::Spacing();
                        ImGui::TextWrapped(description.c_str());
                    }
                }
                ImGui::EndChild();
                ImGui::BeginChild("##Buttons");
                {
                    if (this->selectedAddon > -1) {
                        ImGui::SameLine();
                        if (ImGui::Button("Move Up")) {
                            this->MoveAddonPositionUp(filePath);
                            this->SelectAddon(filePath);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Move Down")) {
                            this->MoveAddonPositionDown(filePath);
                            this->SelectAddon(filePath);
                        }
                        
                        if (state == addons::types::AddonState::LoadedState) {
                            ImGui::SameLine();
                            if (ImGui::Button("Deactivate")) {
                                AppConfig.SetAddonEnabled(fileName, false);
                                addon->Unload();
                                if (addon->GetTypeImpl()->GetAddonState() == addons::types::AddonState::DeactivatedOnRestartState) {
                                    gui::MessageWindow::ShowMessageWindow(
                                        L"Deactivate add-on##ActivationPopup",
                                        L"This add-on cannot be deactivated while Guild Wars 2 is running. A restart is required.");
                                }
                            }
                        }
                        else if (state == addons::types::AddonState::UnloadedState) {
                            ImGui::SameLine();
                            if (ImGui::Button("Activate")) {
                                if (addon->Load()) {
                                    AppConfig.SetAddonEnabled(fileName, false);
                                    auto state = addon->GetTypeImpl()->GetAddonState();
                                    if (state == addons::types::AddonState::LoadedState) {
                                        AppConfig.SetAddonEnabled(fileName, true);
                                    }
                                    else if (state == addons::types::AddonState::ActivatedOnRestartState) {
                                        gui::MessageWindow::ShowMessageWindow(
                                            L"Activate add-on##ActivationPopup",
                                            L"This add-on cannot be activated while Guild Wars 2 is running. A restart is required.");
                                    }
                                } else {
                                    AppConfig.SetAddonEnabled(fileName, false);
                                    addon->Unload();
                                }
                            }
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        void SettingsWindow::MoveAddonPositionUp(const wstring& fileName) {
            int index = -1;
            for (auto it = addons::AddonsList.rbegin(); it != addons::AddonsList.rend(); ++it) {
                if ((*it)->GetFilePath() == fileName) {
                    index = static_cast<int>(it - addons::AddonsList.rbegin());
                }
                else if (index > -1 && (*it)->GetAddonType() != addons::AddonType::UnknownAddon) {
                    AppConfig.SetAddonOrder((*it)->GetFileName(), addons::AddonsList.size() - (index + 1));
                    iter_swap(addons::AddonsList.rbegin() + index, it);
                    AppConfig.SetAddonOrder((*it)->GetFileName(), addons::AddonsList.size() - (index + 2));
                    break;
                }
            }
        }

        void SettingsWindow::MoveAddonPositionDown(const wstring& fileName) {
            int index = -1;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                if ((*it)->GetFilePath() == fileName) {
                    index = static_cast<int>(it - addons::AddonsList.begin());
                }
                else if (index > -1 && (*it)->GetAddonType() != addons::AddonType::UnknownAddon) {
                    AppConfig.SetAddonOrder((*it)->GetFileName(), index);
                    iter_swap(addons::AddonsList.begin() + index, it);
                    AppConfig.SetAddonOrder((*it)->GetFileName(), index + 1);
                    break;
                }
            }
        }

        void SettingsWindow::SelectAddon(const wstring& fileName) {
            int index = -1;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                if ((*it)->GetAddonType() != addons::AddonType::UnknownAddon) {
                    index++;
                }
                if ((*it)->GetFilePath() == fileName) {
                    this->selectedAddon = index;
                }
            }
        }

    }
}
