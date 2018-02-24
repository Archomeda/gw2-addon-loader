#include "SettingsWindow.h"
#include "../windows.h"
#include <shellapi.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <imgui.h>
#include "gui_manager.h"
#include "AddonInfoWindow.h"
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
            this->SetTitle(L"Addon Loader");
            this->SetFlags(ImGuiWindowFlags_NoCollapse);
        }

        void SettingsWindow::Render() {
            ImGui::BeginChild("##Tabs", ImVec2(48, -1), false, ImGuiWindowFlags_NoScrollbar);
            {
                int i = 0;
                this->PushTabStyle(i);
                if (ImGui::Button(ICON_MD_EXTENSION)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Addons");
                }

                this->PushTabStyle(++i);
                if (ImGui::Button(ICON_MD_SHOW_CHART)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Statistics");
                }

                this->PushTabStyle(++i);
                if (ImGui::Button(ICON_MD_SETTINGS)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Settings");
                }

                this->PushTabStyle(++i);
                if (ImGui::Button(ICON_MD_INFO)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Info");
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginChild("##TabPanel", ImVec2(0, 0));
            switch (this->selectedTab) {
            case 0:
                this->RenderTabAddons();
                break;
            case 1:
                this->RenderTabStats();
                break;
            case 2:
                this->RenderTabSettings();
                break;
            case 3:
                this->RenderTabInfo();
                break;
            }
            ImGui::EndChild();
        }


        void SettingsWindow::BeginStyle() {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(680, 380));
        }

        void SettingsWindow::EndStyle() {
            ImGui::PopStyleVar();
        }


        bool SettingsWindow::ImGuiAddonsList(const char* label, int* current_item, const char* const* items, int items_count, const ImVec2& listBoxSize, float listItemHeight) {
            if (!ImGui::ListBoxHeader(label, listBoxSize)) {
                return false;
            }

            ImGuiStyle style = ImGui::GetStyle();
            listItemHeight += style.FramePadding.y * 2;
            bool value_changed = false;
            ImGuiListClipper clipper(items_count, listItemHeight);
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    const bool item_selected = (i == *current_item);
                    const char* item_text = items[i];

                    ImGui::PushID(i);
                    if (ImGui::Selectable("##dummy", item_selected, 0, ImVec2(0, listItemHeight))) {
                        *current_item = i;
                        value_changed = true;
                    }
                    ImVec2 oldPos = ImGui::GetCursorPos();
                    ImGui::SetCursorPosY(oldPos.y - listItemHeight);
                    //TODO: Draw temporary icon here until we have addon specific icons
                    ImGui::PushFont(FontIconButtons);
                    ImGui::Text(ICON_MD_EXTENSION);
                    ImGui::PopFont();
                    ImGui::SetCursorPos(ImVec2(oldPos.x + 32 + style.ItemSpacing.x, oldPos.y - style.ItemSpacing.y - ((listItemHeight + ImGui::GetTextLineHeightWithSpacing()) / 2)));
                    ImGui::Text(item_text);
                    ImGui::SetCursorPos(oldPos);
                    ImGui::PopID();
                }
            }
            ImGui::ListBoxFooter();
            return value_changed;
        }

        void SettingsWindow::RenderTabAddons() {
            ImGuiStyle style = ImGui::GetStyle();
           
            // Filter the list first
            vector<shared_ptr<addons::Addon>> addons;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                if ((*it)->GetAddonType() != addons::AddonType::UnknownAddon) {
                    addons.push_back(*it);
                }
            }

            // Get selected addon info
            int selectedAddon = this->selectedAddon;
            shared_ptr<addons::Addon> addon = nullptr;
            wstring filePath;
            wstring fileName;
            addons::types::AddonState state;
            string stateString;
            string type;
            string id;
            string productName;
            string description;
            wstring homepage;
            if (selectedAddon > -1 && selectedAddon < static_cast<int>(addons.size())) {
                addon = addons[selectedAddon];
                filePath = addon->GetFilePath();
                fileName = addon->GetFileName();
                state = addon->GetTypeImpl()->GetAddonState();
                stateString = ws2s(addon->GetTypeImpl()->GetAddonStateString());
                type = ws2s(addon->GetAddonTypeString());
                id = ws2s(addon->GetID());
                productName = ws2s(addon->GetProductName());
                description = ws2s(addon->GetDescription());
                homepage = addon->GetHomepage();
            }

            // Left side-panel
            ImGui::BeginGroup();
            {
                ImGui::BeginChild("##LeftPanel", ImVec2(200, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Listbox with addons
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
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
                        ImGuiAddonsList("##Addons", &this->selectedAddon, &cnames[0], static_cast<int>(addons.size()), ImVec2(-1, -1), 32);
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndChild();

                // Button group for sorting addons
                if (selectedAddon > -1) {
                    ImVec2 buttonSize((200 - style.ItemSpacing.x) / 2, 0);
                    if (ImGui::Button(ICON_MD_ARROW_UPWARD, buttonSize)) {
                        this->MoveAddonPositionUp(filePath);
                        this->SelectAddon(filePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_MD_ARROW_DOWNWARD, buttonSize)) {
                        this->MoveAddonPositionDown(filePath);
                        this->SelectAddon(filePath);
                    }
                }
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            // Main panel
            if (selectedAddon > -1) {
                ImGui::BeginGroup();
                {
                    // Addon information
                    ImGui::BeginChild("##Addon", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
                    {
                        // Header
                        ImGui::BeginGroup();
                        {
                            if (!productName.empty()) {
                                ImGui::Text(productName.c_str());
                            }
                            else {
                                ImGui::Text(id.c_str());
                            }
                        }
                        ImGui::EndGroup();
                        ImGui::Separator();

                        // Other stuff
                        if (!id.empty()) {
                            ImGui::Text("Status:");
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

                    ImGui::BeginChild("##AddonButtons");
                    {
                        // Activate / deactivate button
                        if (state == addons::types::AddonState::LoadedState) {
                            if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Deactivate", ImVec2(100, 0))) {
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
                            if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Activate", ImVec2(100, 0))) {
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
                                }
                                else {
                                    AppConfig.SetAddonEnabled(fileName, false);
                                    addon->Unload();
                                }
                            }
                        }
                        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 256 - (2 * style.ItemSpacing.x) - 8); // -8 for the resize grip

                        // Settings button
                        if (ImGui::Button(ICON_MD_SETTINGS " Settings", ImVec2(80, 0))) {
                        }
                        ImGui::SameLine();

                        // Info button
                        if (ImGui::Button(ICON_MD_INFO " Details", ImVec2(80, 0))) {
                            AddonInfoWnd->SetSelectedAddon(this->selectedAddon);
                            ShowWindow(AddonInfoWnd);
                        }
                        ImGui::SameLine();

                        // Homepage button
                        if (ImGui::Button(ICON_MD_HOME " Homepage", ImVec2(96, 0))) {
                            ShellExecute(0, 0, homepage.c_str(), 0, 0, SW_SHOW);
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndGroup();
            }
            else {

            }
        }

        void SettingsWindow::RenderTabInfo() {
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(R"(Guild Wars 2 Addon Loader is an unofficial addon for Guild Wars 2 developed by Archomeda. It acts as a master addon that makes it easy to manage your Guild Wars 2 addons without the need to sort the files manually.

Addons have to be compatible before they can be used with the Addon Loader. While unsupported addons will still show up in the list of addons, you can't activate them. If you want to use an addon that's not supported, ask its developer to make it compatible.

Both this addon and its author do not support addons that are against the Guild Wars 2 Terms of Service. Support will not be provided if you're using these kind of addons. Please do not use them.

Visit the GitHub repository for more information, third party licenses, and report any issues related to the Addon Loader there. Keep in mind that the Addon Loader has no influence on what other addons do. Therefore if there are issues related to a specific addon, please report that issue to its developer instead.


Disclaimer:
The author of this library is not associated with ArenaNet nor with any of its partners. Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners. By using this software, you agree that it is at your own risk and that you assume all responsibility. There is no warranty for using this software.)");
            ImGui::PopTextWrapPos();

            ImGui::Dummy(ImVec2(0, 16));
            if (ImGui::Button("Visit GitHub")) {
                ShellExecute(0, 0, L"https://github.com/Archomeda/gw2-addon-loader", 0, 0, SW_SHOW);
            }
        }

        void SettingsWindow::RenderTabSettings() {

        }

        void SettingsWindow::RenderTabStats() {

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


        void SettingsWindow::PushTabStyle(int tabIndex) {
            ImGuiStyle* style = &ImGui::GetStyle();
            if (this->selectedTab == tabIndex) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, style->Colors[ImGuiCol_Button].w));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            }
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, style->Colors[ImGuiCol_ButtonHovered].w));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, style->Colors[ImGuiCol_ButtonActive].w));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.69f, 0.69f, 0.46f, 1.00f));
            ImGui::PushFont(FontIconButtons);
        }

        void SettingsWindow::PopTabStyle(int tabIndex) {
            ImGui::PopStyleColor(4);
            ImGui::PopFont();
        }

    }
}
