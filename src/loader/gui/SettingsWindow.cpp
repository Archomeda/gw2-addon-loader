#include "SettingsWindow.h"
#include "../windows.h"
#include <shellapi.h>
#include <chrono>
#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>
#include <IconsMaterialDesign.h>
#include "imgui.h"
#include <imgui_internal.h>
#include "gui_manager.h"
#include "AddonInfoWindow.h"
#include "MessageWindow.h"
#include "../IconsOcticons.h"
#include "../addons/addons_manager.h"
#include "../addons/Addon.h"
#include "../hooks/LoaderDirect3DDevice9.h"
#include "../hooks/MumbleLink.h"
#include "../Config.h"
#include "../input.h"
#include "../version.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::experimental::filesystem::v1;
using namespace loader::addons;
using namespace loader::utils;

namespace loader {
    namespace gui {

        SettingsWindow::SettingsWindow() {
            this->SetTitle("Addon Loader");
            this->SetFlags(ImGuiWindowFlags_NoCollapse);
        }

        void SettingsWindow::Render() {
            if (!this->initializedState) {
                this->windowKeybind = AppConfig.GetSettingsKeybind();
                this->obsCompatibilityMode = AppConfig.GetOBSCompatibilityMode();
                this->showUnsupportedAddons = AppConfig.GetShowUnsupportedAddons();
                this->showDebugFeatures = AppConfig.GetShowDebugFeatures();
                this->initializedState = true;
            }

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

                if (AppConfig.GetShowDebugFeatures()) {
                    this->PushTabStyle(++i);
                    if (ImGui::Button(ICON_MD_SHOW_CHART)) {
                        this->selectedTab = i;
                    }
                    this->PopTabStyle(i);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Statistics");
                    }
                }
                else {
                    ++i;
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

                string latestVersion = AppConfig.GetLastestVersion();
                if (!latestVersion.empty() && latestVersion != VERSION) {
                    this->PushTabStyle(++i);
                    if (ImGui::Button(ICON_MD_FILE_DOWNLOAD)) {
                        this->selectedTab = i;
                    }
                    this->PopTabStyle(i);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Download update");
                    }
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
            case 4:
                this->RenderTabUpdate();
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


        bool SettingsWindow::ImGuiAddonsList(const char* label, int* current_item, const vector<Addon*> addons, const ImVec2& listBoxSize, float listItemHeight) {
            if (!ImGui::ListBoxHeader(label, listBoxSize)) {
                return false;
            }

            ImGuiStyle style = ImGui::GetStyle();
            listItemHeight += style.FramePadding.y * 2;
            bool value_changed = false;
            ImGuiListClipper clipper(static_cast<int>(addons.size()), listItemHeight);
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    const bool item_selected = (i == *current_item);
                    auto addon = addons.at(i);

                    ImGui::PushID(i);
                    if (ImGui::Selectable("##dummy", item_selected, 0, ImVec2(0, listItemHeight))) {
                        *current_item = i;
                        value_changed = true;
                    }

                    ImVec2 oldPos = ImGui::GetCursorPos();
                    ImGui::SetCursorPosY(oldPos.y - listItemHeight);
                    IDirect3DTexture9* icon = addon->GetIcon();
                    if (icon) {
                        // Dedicated addon icon
                        ImGui::Image(icon, ImVec2(32, 32));
                    }
                    else {
                        // Fallback generic icon
                        ImGui::PushFont(imgui::FontIconButtons);
                        if (addon->SupportsLoading()) {
                            ImGui::Text(ICON_MD_EXTENSION);
                        }
                        ImGui::PopFont();
                    }
                    ImGui::SetCursorPos(ImVec2(oldPos.x + 32 + style.ItemSpacing.x, oldPos.y - style.ItemSpacing.y - ((listItemHeight + ImGui::GetTextLineHeightWithSpacing()) / 2)));
                    if (addon->SupportsLoading()) {
                        ImGui::Text(addon->GetName().c_str());
                    }
                    else {
                        ImGui::TextDisabled(addon->GetName().c_str());
                    }
                    ImGui::SetCursorPos(oldPos);
                    ImGui::PopID();
                }
            }
            ImGui::ListBoxFooter();
            return value_changed;
        }

        void SettingsWindow::ImGuiAddonStatLine(const char* label, const TimeMeasure& measure, bool calls = true) {
            ImGui::TextUnformatted(label);
            ImGui::NextColumn();
            if (measure.GetLast() >= 500) {
                ImGui::TextColored(ImVec4(0.75, 0.5625f, 0.375f, 1.0f), "%.0f µs", measure.GetLast());
            }
            else {
                ImGui::Text("%.0f µs", measure.GetLast());
            }
            ImGui::NextColumn();
            if (measure.GetMovingAverage() >= 500) {
                ImGui::TextColored(ImVec4(0.75, 0.5625f, 0.375f, 1.0f), "%.0f µs", measure.GetMovingAverage());
            }
            else {
                ImGui::Text("%.0f µs", measure.GetMovingAverage());
            }
            ImGui::NextColumn();
            ImGui::Text("%.0f µs", measure.GetOverallMaximum());
            ImGui::NextColumn();
            if (calls) {
                ImGui::Text("%llu", measure.GetCalls());
            }
            ImGui::NextColumn();
        }


        void SettingsWindow::RenderTabAddons() {
            ImGuiStyle style = ImGui::GetStyle();
           
            vector<Addon*> addonsList;
            for (auto addon : AddonsList) {
                if (AppConfig.GetShowUnsupportedAddons() || addon->SupportsLoading()) {
                    addonsList.push_back(addon.get());
                }
                else {
                    break;
                }
            }

            // Get selected addon info
            Addon* addon = nullptr;
            if (this->selectedAddon >= static_cast<int>(addonsList.size())) {
                this->selectedAddon = -1;
            }

            // Left side-panel
            ImGui::BeginGroup();
            {
                ImGui::BeginChild("##LeftPanel", ImVec2(200, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Listbox with addons
                    if (addonsList.size() > 0) {
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
                        ImGuiAddonsList("##Addons", &this->selectedAddon, addonsList, ImVec2(-1, -1), 32);
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndChild();
                
                if (this->selectedAddon > -1 && this->selectedAddon < static_cast<int>(addonsList.size())) {
                    // Set the selected addon after the listbox, in case the listbox selection has changed in this frame
                    addon = addonsList[this->selectedAddon];
                }

                // Button group for sorting addons
                ImVec2 buttonSize((200 - style.ItemSpacing.x) / 2, 0);
                if (this->selectedAddon > 0 && this->selectedAddon < static_cast<int>(addonsList.size()) && addon->SupportsLoading()) {
                    if (ImGui::Button(ICON_MD_ARROW_UPWARD, buttonSize)) {
                        this->MoveAddonPositionUp(addon);
                        this->SelectAddon(addon);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Move selected addon up");
                    }
                }
                else {
                    ImGui::Dummy(buttonSize);
                }
                ImGui::SameLine();
                if (this->selectedAddon > -1 && this->selectedAddon < static_cast<int>(addonsList.size()) - 1 && addonsList.at(this->selectedAddon + 1)->SupportsLoading()) {
                    if (ImGui::Button(ICON_MD_ARROW_DOWNWARD, buttonSize)) {
                        this->MoveAddonPositionDown(addon);
                        this->SelectAddon(addon);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Move selected addon down");
                    }
                }
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            // Main panel
            if (this->selectedAddon > -1) {
                ImGui::BeginGroup();
                {
                    // Addon information
                    ImGui::BeginChild("##Addon", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
                    {
                        // Header
                        ImGui::Text((addon->GetName() + " " + addon->GetVersion()).c_str());
                        ImGui::Separator();

                        // Other stuff
                        if (addon->SupportsLoading()) {
                            ImGui::Text("Status:");
                            ImGui::SameLine();
                            ImColor color(120, 120, 120);
                            switch (addon->GetState()) {
                            case AddonState::LoadingState:
                                color = ImColor(255, 255, 0);
                                break;
                            case AddonState::ActivatedOnRestartState:
                            case AddonState::DeactivatedOnRestartState:
                                color = ImColor(0, 255, 255);
                                break;
                            case AddonState::LoadedState:
                                color = ImColor(0, 255, 0);
                                break;
                            case AddonState::ErroredState:
                                color = ImColor(255, 0, 0);
                                break;
                            }
                            ImGui::TextColored(color, addon->GetStateString().c_str());
                        }
                        else {
                            ImGui::PushTextWrapPos();
                            ImGui::Text("This addon type is not supported by the addon loader: %s.", addon->GetTypeString().c_str());
                            ImGui::PopTextWrapPos();
                        }

                        if (!addon->GetDescription().empty()) {
                            ImGui::Spacing();
                            ImGui::TextWrapped(addon->GetDescription().c_str());
                        }
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild("##AddonButtons");
                    {
                        if (addon->SupportsLoading()) {
                            // Activate / deactivate button
                            if (addon->GetState() == AddonState::LoadedState) {
                                if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Deactivate", ImVec2(100, 0))) {
                                    AppConfig.SetAddonEnabled(addon, false);
                                    addon->Unload();
                                    if (addon->GetState() == AddonState::DeactivatedOnRestartState) {
                                        gui::MessageWindow::ShowMessageWindow(
                                            "Deactivate add-on##ActivationPopup",
                                            "This add-on cannot be deactivated while Guild Wars 2 is running. A restart is required.");
                                    }
                                }
                            }
                            else if (addon->GetState() == AddonState::UnloadedState) {
                                if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Activate", ImVec2(100, 0))) {
                                    if (addon->Load()) {
                                        AppConfig.SetAddonEnabled(addon, false);
                                        auto state = addon->GetState();
                                        if (state == AddonState::LoadedState) {
                                            AppConfig.SetAddonEnabled(addon, true);
                                        }
                                        else if (state == AddonState::ActivatedOnRestartState) {
                                            gui::MessageWindow::ShowMessageWindow(
                                                "Activate add-on##ActivationPopup",
                                                "This add-on cannot be activated while Guild Wars 2 is running. A restart is required.");
                                        }
                                    }
                                    else {
                                        AppConfig.SetAddonEnabled(addon, false);
                                        addon->Unload();
                                    }
                                }
                            }
                        }

                        // Settings button
                        if (addon->IsLoaded() && addon->SupportsSettings()) {
                            ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 256 - (2 * style.ItemSpacing.x) - 8); // -8 for the resize grip
                            if (ImGui::Button(ICON_MD_SETTINGS " Settings", ImVec2(80, 0))) {
                                // Close ourselves and show the addon settings
                                this->Close();
                                addon->OpenSettings();
                            }
                        }

                        // Info button
                        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 176 - style.ItemSpacing.x - 8); // -8 for the resize grip
                        if (ImGui::Button(ICON_MD_INFO " Details", ImVec2(80, 0))) {
                            AddonInfoWnd->SetAddon(addon);
                            ShowWindow(AddonInfoWnd);
                        }

                        // Homepage button
                        if (addon->SupportsHomepage()) {
                            ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 96 - 8); // -8 for the resize grip
                            if (ImGui::Button(ICON_MD_HOME " Homepage", ImVec2(96, 0))) {
                                wstring wHomepage = u16(addon->GetHomepage());
                                ShellExecute(0, 0, wHomepage.c_str(), 0, 0, SW_SHOW);
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndGroup();
            }
            else if (addonsList.size() > 0) {
                ImGui::PushTextWrapPos();
                ImGui::TextUnformatted("No addon selected. Select an addon in the list to the left.");
                ImGui::PopTextWrapPos();
            }
            else {
                ImGui::PushTextWrapPos();
                ImGui::TextUnformatted("No addons available. Install at least one addon and restart Guild Wars 2.");
                ImGui::PopTextWrapPos();
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
            if (ImGui::Button(ICON_OC_MARK_GITHUB "  " ICON_OC_LOGO_GITHUB, ImVec2(90, 32))) {
                ShellExecute(0, 0, L"https://github.com/Archomeda/gw2-addon-loader", 0, 0, SW_SHOW);
            }
        }

        void SettingsWindow::RenderTabSettings() {
            ImGui::BeginChild("##Settings", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
            {
                set<uint_fast8_t> pressedKeys = GetPressedKeyboardKeys();
                string keysStr = GetReadableKeyString(this->windowKeybindEditActive ? pressedKeys : this->windowKeybind);
                char keysBuff[64];
                keysStr._Copy_s(keysBuff, sizeof(keysBuff), keysStr.length());
                keysBuff[keysStr.length()] = 0;
                ImGui::TextUnformatted("Addon Loader Window keybind");
                ImGui::SameLine();
                ImGui::InputTextEx("##LoaderKeybind", keysBuff, sizeof(keysBuff), ImVec2(200, 0), ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemActive()) {
                    this->windowKeybindEditActive = true;
                    if (DoKeysContainNonModifiers(pressedKeys)) {
                        // Apply keybind
                        ImGui::ClearActiveID();
                        this->windowKeybind = pressedKeys;
                        AppConfig.SetSettingsKeybind(pressedKeys);
                        this->windowKeybindEditActive = false;
                    }
                }
                else if (this->windowKeybindEditActive) {
                    // Reset keybind
                    this->windowKeybind = AppConfig.GetSettingsKeybind();
                    this->windowKeybindEditActive = false;
                }
                else if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to activate the field and press a new keybind. Use Escape to cancel.");
                }

                if (ImGui::Checkbox("Enable OBS compatibility mode", &this->obsCompatibilityMode)) {
                    AppConfig.SetOBSCompatibilityMode(this->obsCompatibilityMode);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("When OBS compatibility mode is active, the addon loader will attempt to hide\nitself from OBS if the OBS third-party overlays capture setting is disabled.");
                }

                if (ImGui::Checkbox("Show unsupported addons", &this->showUnsupportedAddons)) {
                    AppConfig.SetShowUnsupportedAddons(this->showUnsupportedAddons);
                }

                if (ImGui::Checkbox("Show debug features", &this->showDebugFeatures)) {
                    AppConfig.SetShowDebugFeatures(this->showDebugFeatures);
                }
            }
            ImGui::EndChild();
         
            ImGui::BeginChild("##Metadata");
            {
                ImGui::Text("Addon Loader version: " VERSION);
            }
            ImGui::EndChild();
        }

        void SettingsWindow::RenderTabStats() {
            shared_ptr<Addon> selectedAddon;
            stringstream sstream;
            sstream << "Guild Wars 2" << '\0' << "Addon Loader" << '\0';
            int i = 2;
            for (const auto& addon : AddonsList) {
                if (addon->GetState() == AddonState::LoadedState) {
                    sstream << addon->GetName() << '\0';
                    if (this->selectedStatsType == i) {
                        selectedAddon = addon;
                    }
                    ++i;
                }
            }

            ImGui::PushItemWidth(-1);
            string types = sstream.str();
            ImGui::Combo("##RenderType", &this->selectedStatsType, types.c_str());

            if (this->selectedStatsType == 0) {
                ImGui::PlotLines("##RenderingTime", &hooks::DurationHistoryD3D9Processing[0], static_cast<int>(hooks::DurationHistoryD3D9Processing.size()), 0, "Frame render time (ms)", 0, 100, ImVec2(0, 200));

                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImGuiStyle style = ImGui::GetStyle();
                ImRect plotRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                float yFps30 = plotRect.Max.y - (plotRect.GetSize().y / 3.0f);
                float yFps60 = plotRect.Max.y - (plotRect.GetSize().y / 6.0f);
                draw->AddLine(ImVec2(plotRect.Min.x, yFps30), ImVec2(plotRect.Max.x, yFps30), IM_COL32(192, 144, 96, 255));
                draw->AddText(ImVec2(plotRect.Min.x + 2, yFps30 + 1), IM_COL32(192, 144, 96, 255), "30fps");
                draw->AddLine(ImVec2(plotRect.Min.x, yFps60), ImVec2(plotRect.Max.x, yFps60), IM_COL32(96, 192, 96, 255));
                draw->AddText(ImVec2(plotRect.Min.x + 2, yFps60 + 1), IM_COL32(96, 192, 96, 255), "60fps");
                ImGui::Spacing();

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 180);
                ImGui::TextUnformatted("Map type");
                ImGui::NextColumn();
                ImGui::TextUnformatted(hooks::MapTypeToString(hooks::Gw2MumbleLink.GetMapType()).c_str());
                ImGui::NextColumn();
                ImGui::TextUnformatted("Map ID");
                ImGui::NextColumn();
                ImGui::TextUnformatted(to_string(hooks::Gw2MumbleLink.GetMapId()).c_str());
                ImGui::Columns(1);
            }
            else if (this->selectedStatsType == 1) {
                ImGui::PlotLines("##RenderingTime", &hooks::DurationHistoryLoaderDrawFrame[0], static_cast<int>(hooks::DurationHistoryLoaderDrawFrame.size()), 0, "Frame render time (ms)", 0, 4, ImVec2(0, 70));
            }
            else if (selectedAddon) {
                if (ImGui::CollapsingHeader("General")) {
                    ImGui::Columns(5);
                    ImGui::SetColumnWidth(0, 180);
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Current");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Exp. moving avg");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Overall max");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Calls");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Load");
                    ImGui::NextColumn();
                    ImGui::Text("%.0f µs", selectedAddon->GetLoadDuration());
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    this->ImGuiAddonStatLine("WndProc", selectedAddon->GetTimeWndProc());

                    ImGui::Columns(1);
                }
                if (ImGui::CollapsingHeader("Rendering")) {
                    const auto history = selectedAddon->GetTimeOverall().GetMovingHistory();
                    ImGui::PlotLines("##RenderingTime", &history[0], static_cast<int>(history.size()), 0, "Addon frame time (µs)", 0, 10000, ImVec2(0, 100));
                    ImGui::Columns(5);
                    ImGui::SetColumnWidth(0, 180);
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Current");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Exp. moving avg");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Overall max");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Calls");
                    ImGui::NextColumn();

                    this->ImGuiAddonStatLine("Total", selectedAddon->GetTimeOverall(), false);
                    this->ImGuiAddonStatLine("DrawBeforePostProcessing", selectedAddon->GetTimeDrawFrameBeforePostProcessing());
                    this->ImGuiAddonStatLine("DrawBeforeGui", selectedAddon->GetTimeDrawFrameBeforeGui());
                    this->ImGuiAddonStatLine("Draw", selectedAddon->GetTimeDrawFrame());
                    this->ImGuiAddonStatLine("AdvPreBeginScene", selectedAddon->GetTimeAdvPreBeginScene());
                    this->ImGuiAddonStatLine("AdvPostBeginScene", selectedAddon->GetTimeAdvPostBeginScene());
                    this->ImGuiAddonStatLine("AdvPreEndScene", selectedAddon->GetTimeAdvPreEndScene());
                    this->ImGuiAddonStatLine("AdvPostEndScene", selectedAddon->GetTimeAdvPostEndScene());
                    this->ImGuiAddonStatLine("AdvPreClear", selectedAddon->GetTimeAdvPreClear());
                    this->ImGuiAddonStatLine("AdvPostClear", selectedAddon->GetTimeAdvPostClear());
                    this->ImGuiAddonStatLine("AdvPreReset", selectedAddon->GetTimeAdvPreReset());
                    this->ImGuiAddonStatLine("AdvPostReset", selectedAddon->GetTimeAdvPostReset());
                    this->ImGuiAddonStatLine("AdvPrePresent", selectedAddon->GetTimeAdvPrePresent());
                    this->ImGuiAddonStatLine("AdvPostPresent", selectedAddon->GetTimeAdvPostPresent());
                    this->ImGuiAddonStatLine("AdvPreCreateTexture", selectedAddon->GetTimeAdvPreCreateTexture());
                    this->ImGuiAddonStatLine("AdvPostCreateTexture", selectedAddon->GetTimeAdvPostCreateTexture());
                    this->ImGuiAddonStatLine("AdvPreCreateVertexShader", selectedAddon->GetTimeAdvPreCreateVertexShader());
                    this->ImGuiAddonStatLine("AdvPostCreateVertexShader", selectedAddon->GetTimeAdvPostCreateVertexShader());
                    this->ImGuiAddonStatLine("AdvPreCreatePixelShader", selectedAddon->GetTimeAdvPreCreatePixelShader());
                    this->ImGuiAddonStatLine("AdvPostCreatePixelShader", selectedAddon->GetTimeAdvPostCreatePixelShader());
                    this->ImGuiAddonStatLine("AdvPreCreateRenderTarget", selectedAddon->GetTimeAdvPreCreateRenderTarget());
                    this->ImGuiAddonStatLine("AdvPostCreateRenderTarget", selectedAddon->GetTimeAdvPostCreateRenderTarget());
                    this->ImGuiAddonStatLine("AdvPreSetTexture", selectedAddon->GetTimeAdvPreSetTexture());
                    this->ImGuiAddonStatLine("AdvPostSetTexture", selectedAddon->GetTimeAdvPostSetTexture());
                    this->ImGuiAddonStatLine("AdvPreSetVertexShader", selectedAddon->GetTimeAdvPreSetVertexShader());
                    this->ImGuiAddonStatLine("AdvPostSetVertexShader", selectedAddon->GetTimeAdvPostSetVertexShader());
                    this->ImGuiAddonStatLine("AdvPreSetPixelShader", selectedAddon->GetTimeAdvPreSetPixelShader());
                    this->ImGuiAddonStatLine("AdvPostSetPixelShader", selectedAddon->GetTimeAdvPostSetPixelShader());
                    this->ImGuiAddonStatLine("AdvPreSetRenderTarget", selectedAddon->GetTimeAdvPreSetRenderTarget());
                    this->ImGuiAddonStatLine("AdvPostSetRenderTarget", selectedAddon->GetTimeAdvPostSetRenderTarget());
                    this->ImGuiAddonStatLine("AdvPreSetRenderState", selectedAddon->GetTimeAdvPreSetRenderState());
                    this->ImGuiAddonStatLine("AdvPostSetRenderState", selectedAddon->GetTimeAdvPostSetRenderState());
                    this->ImGuiAddonStatLine("AdvPreDrawIndexedPrimitive", selectedAddon->GetTimeAdvPreDrawIndexedPrimitive());
                    this->ImGuiAddonStatLine("AdvPostDrawIndexedPrimitive", selectedAddon->GetTimeAdvPostDrawIndexedPrimitive());
                   
                    ImGui::Columns(1);
                }
            }
            
            ImGui::PopItemWidth();
        }

        void SettingsWindow::RenderTabUpdate() {
            ImGui::BeginChild("##Update");
            {
                ImGui::Text("Current version: %s", VERSION);
                ImGui::Text("Latest version: %s", AppConfig.GetLastestVersion().c_str());
                ImGui::Dummy(ImVec2(0, 16));
                if (ImGui::Button(ICON_OC_MARK_GITHUB "   Download from GitHub", ImVec2(175, 32))) {
                    ShellExecute(0, 0, u16(AppConfig.GetLastestVersionInfoUrl()).c_str(), 0, 0, SW_SHOW);
                }
            }
            ImGui::EndChild();
        }


        void SettingsWindow::MoveAddonPositionUp(const Addon* const addon) {
            int index = -1;
            for (auto it = addons::AddonsList.rbegin(); it != addons::AddonsList.rend(); ++it) {
                if ((*it)->GetID() == addon->GetID()) {
                    index = static_cast<int>(it - addons::AddonsList.rbegin());
                }
                else if (index > -1) {
                    AppConfig.SetAddonOrder(it->get(), static_cast<int>(addons::AddonsList.size() - (index + 1)));
                    iter_swap(addons::AddonsList.rbegin() + index, it);
                    AppConfig.SetAddonOrder(it->get(), static_cast<int>(addons::AddonsList.size() - (index + 2)));
                    break;
                }
            }
        }

        void SettingsWindow::MoveAddonPositionDown(const Addon* const addon) {
            int index = -1;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                if ((*it)->GetID() == addon->GetID()) {
                    index = static_cast<int>(it - addons::AddonsList.begin());
                }
                else if (index > -1) {
                    AppConfig.SetAddonOrder(it->get(), index);
                    iter_swap(addons::AddonsList.begin() + index, it);
                    AppConfig.SetAddonOrder(it->get(), index + 1);
                    break;
                }
            }
        }

        void SettingsWindow::SelectAddon(const Addon* const addon) {
            int index = -1;
            for (auto it = addons::AddonsList.begin(); it != addons::AddonsList.end(); ++it) {
                index++;
                if ((*it)->GetID() == addon->GetID()) {
                    this->selectedAddon = index;
                    break;
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.89f, 0.88f, 0.68f, 1.00f));
            ImGui::PushFont(imgui::FontIconButtons);
        }

        void SettingsWindow::PopTabStyle(int tabIndex) {
            ImGui::PopStyleColor(4);
            ImGui::PopFont();
        }

    }
}
