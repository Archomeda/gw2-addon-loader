#include "SettingsWindow.h"
#include "imgui.h"
#include "gui_manager.h"
#include "elements/ExtraImGuiElements.h"
#include "../Config.h"
#include "../IconsOcticons.h"
#include "../input.h"
#include "../log.h"
#include "../version.h"
#include "../addons/addons_manager.h"
#include "../addons/Addon.h"
#include "../diagnostics/ThreadMonitor.h"
#include "../hooks/LoaderDirect3DDevice9.h"
#include "../hooks/MumbleLink.h"
#include "../updaters/update_manager.h"
#include "../utils/encoding.h"

using namespace std;
using namespace std::filesystem;
using namespace loader::addons;
using namespace loader::diagnostics;
using namespace loader::updaters;
using namespace loader::utils;

namespace loader::gui {

    SettingsWindow::SettingsWindow() {
        this->SetTitle("Add-on Loader");
        this->SetFlags(ImGuiWindowFlags_NoCollapse);
    }

    void SettingsWindow::Render() {
        if (!this->initializedState) {
            this->windowKeybind = AppConfig.GetSettingsKeybind();
            this->obsCompatibilityMode = AppConfig.GetOBSCompatibilityMode();
            this->showHiddenAddons = AppConfig.GetShowHiddenAddons();
            this->diagnostics = AppConfig.GetDiagnostics();
            strcpy_s(this->apiKey, sizeof(this->apiKey), AppConfig.GetApiKey().c_str());
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
                ImGui::SetTooltip("Add-ons");
            }

            if (AppConfig.GetDiagnostics()) {
                this->PushTabStyle(++i);
                if (ImGui::Button(ICON_MD_SHOW_CHART)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Diagnostics");
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

            if (this->IsAddonLoaderUpdateAvailable()) {
                this->PushTabStyle(++i);
                if (ImGui::Button(ICON_MD_FILE_DOWNLOAD)) {
                    this->selectedTab = i;
                }
                this->PopTabStyle(i);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Add-on Loader update available");
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
            this->RenderTabDiagnostics();
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(600, 300));
    }

    void SettingsWindow::EndStyle() {
        ImGui::PopStyleVar();
    }


    bool SettingsWindow::ImGuiAddonsList(const char* label, int* current_item, const vector<shared_ptr<Addon>> addons, const ImVec2& listBoxSize, float listItemHeight) {
        if (!ImGui::ListBoxHeader(label, listBoxSize)) {
            return false;
        }

        ImGuiStyle style = ImGui::GetStyle();
        bool value_changed = false;
        ImGuiListClipper clipper(static_cast<int>(addons.size()), listItemHeight);
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                const bool item_selected = (i == *current_item);
                auto addon = addons[i];

                ImGui::PushID(i);
                if (elements::AddonListItem(addon.get(), item_selected, ImVec2(0, listItemHeight))) {
                    *current_item = i;
                    value_changed = true;
                }
                ImGui::PopID();
            }
        }
        ImGui::ListBoxFooter();
        return value_changed;
    }

    void SettingsWindow::ImGuiAddonStatLine(const char* label, const AddonMetric& measure, bool calls = true) {
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        if (measure.GetLast() >= 500) {
            ImGui::TextColored(ImVec4(0.75, 0.5625f, 0.375f, 1.0f), measure.FormatTimeMetric(measure.GetLast()).c_str());
        }
        else {
            ImGui::TextUnformatted(measure.FormatTimeMetric(measure.GetLast()).c_str());
        }
        ImGui::NextColumn();
        if (measure.GetMovingAverage() >= 500) {
            ImGui::TextColored(ImVec4(0.75, 0.5625f, 0.375f, 1.0f), measure.FormatTimeMetric(measure.GetMovingAverage()).c_str());
        }
        else {
            ImGui::TextUnformatted(measure.FormatTimeMetric(measure.GetMovingAverage()).c_str());
        }
        ImGui::NextColumn();
        ImGui::TextUnformatted(measure.FormatTimeMetric(measure.GetOverallMaximum()).c_str());
        ImGui::NextColumn();
        if (calls) {
            ImGui::Text("%llu", measure.GetCalls());
        }
        ImGui::NextColumn();
    }


    void SettingsWindow::RenderTabAddons() {
        ImGuiStyle& style = ImGui::GetStyle();

        vector<shared_ptr<Addon>> addonsList;
        for (const auto& addon : Addons) {
            if (AppConfig.GetShowHiddenAddons() || !addon->IsHidden()) {
                addonsList.push_back(addon);
            }
        }

        // Get selected add-on info
        shared_ptr<Addon> addon = nullptr;
        if (this->selectedAddon >= static_cast<int>(addonsList.size())) {
            this->selectedAddon = -1;
        }

        // Left side-panel
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("##LeftPanel", ImVec2(200, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                // Listbox with add-ons
                if (addonsList.size() > 0) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
                    ImGuiAddonsList("##Addons", &this->selectedAddon, addonsList, ImVec2(-1, -1), 32);
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();

            if (this->selectedAddon > -1 && this->selectedAddon < static_cast<int>(addonsList.size())) {
                // Set the selected add-on after the listbox, in case the listbox selection has changed in this frame
                addon = addonsList[this->selectedAddon];
            }

            // Button group for refreshing and sorting add-ons
            ImVec2 buttonSize((200 - (2 * style.ItemSpacing.x)) / 3, 0);
            if (ImGui::Button(ICON_MD_REFRESH, buttonSize)) {
                this->selectedAddon = -1;
                RefreshAddonList();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Refresh add-ons");
            }
            ImGui::SameLine();
            if (this->selectedAddon > 0 && Addons.CanSwap(addon.get(), addonsList[this->selectedAddon - 1].get())) {
                if (ImGui::Button(ICON_MD_ARROW_UPWARD, buttonSize)) {
                    SwapAddonOrder(addon.get(), addonsList[this->selectedAddon - 1].get());
                    this->SelectAddon(addon.get());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Move selected add-on up");
                }
            }
            else {
                ImGui::Dummy(buttonSize);
            }
            ImGui::SameLine();
            if (this->selectedAddon > -1 && this->selectedAddon < static_cast<int>(addonsList.size() - 1) && Addons.CanSwap(addon.get(), addonsList[this->selectedAddon + 1].get())) {
                if (ImGui::Button(ICON_MD_ARROW_DOWNWARD, buttonSize)) {
                    SwapAddonOrder(addon.get(), addonsList[this->selectedAddon + 1].get());
                    this->SelectAddon(addon.get());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Move selected add-on down");
                }
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        // Main panel
        if (this->selectedAddon > -1) {
            ImGui::BeginGroup();
            {
                // Add-on information
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
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 51, 0, 255));
                        ImGui::TextWrapped("This add-on type is not compatible: %s.", addon->GetTypeString().c_str());
                        ImGui::PopStyleColor();
                    }

                    if (addon->GetState() == AddonState::DeactivatedOnRestartState) {
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 204, 0, 255));
                        ImGui::TextWrapped("This add-on cannot be deactivated while Guild Wars 2 is running. A full Guild Wars 2 client restart is required.");
                        ImGui::PopStyleColor();
                    }
                    else if (addon->GetState() == AddonState::ActivatedOnRestartState) {
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 204, 0, 255));
                        ImGui::TextWrapped("This add-on cannot be activated while Guild Wars 2 is running. A full Guild Wars 2 client restart is required.");
                        ImGui::PopStyleColor();
                    }

                    if (!addon->GetDescription().empty()) {
                        ImGui::Spacing();
                        ImGui::TextWrapped(addon->GetDescription().c_str());
                    }

                    if (addon->IsForced()) {
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 204, 0, 255));
                        ImGui::TextWrapped("This is a supportive add-on for the Addon Loader and cannot be disabled.");
                        ImGui::PopStyleColor();
                    }
                    else if (addon->GetType() == AddonType::AddonTypeLegacy) {
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 204, 0, 255));
                        ImGui::TextWrapped(ICON_MD_WARNING " WARNING " ICON_MD_WARNING "\nThis is a legacy add-on. Support for legacy add-ons is experimental at best and crashes may happen. Please contact the add-on developer to make it properly compatible.");
                        ImGui::PopStyleColor();
                    }

                    // Potential available update
                    if (addon->HasUpdate()) {
                        VersionInfo version = addon->GetLatestVersion();
                        shared_ptr<Installer> installer = AddonUpdateInstallers[addon.get()];

                        ImGui::Dummy(ImVec2(0, 8));
                        ImGui::PushTextWrapPos();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(153, 238, 238, 255));
                        ImGui::TextUnformatted("An update is available for this add-on.\n" ICON_MD_WARNING " Disclaimer: Be careful; updating add-ons automatically is based on trust. The download is provided by the add-on itself and not by the Add-on Loader. Always check the release notes first. There is no guarantee that a newer version doesn't contain additional code that might harm your Guild Wars 2 account or your computer. After the update has finished, a full Guild Wars 2 restart may be required.");
                        ImGui::PopStyleColor();
                        ImGui::PopTextWrapPos();

                        ImGui::Dummy(ImVec2(0, 4));
                        ImGui::Text("Installed version: %s", addon->GetVersion().c_str());
                        ImGui::Text("New version: %s", version.version.c_str());
                        ImGui::Dummy(ImVec2(0, 4));
                        if (!version.infoUrl.empty()) {
                            if (ImGui::Button("Open release notes", ImVec2(160, 32))) {
                                ShellExecute(0, 0, u16(version.infoUrl).c_str(), 0, 0, SW_SHOW);
                            }
                        }
                        ImGui::SameLine();
                        if (installer != nullptr && (installer->IsActive() || installer->HasCompleted())) {
                            ImGui::ProgressBar(installer->GetProgressFraction(), ImVec2(-1, 32), installer->GetDetailedProgress().c_str());
                        }
                        else {
                            if (ImGui::Button(ICON_MD_FILE_DOWNLOAD " Update", ImVec2(100, 32))) {
                                InstallUpdate(addon);
                            }
                        }
                    }

                    // Some metadata
                    ImGui::Spacing();
                    ImGui::Separator();
                    if (!addon->GetID().empty()) {
                        ImGui::Text("ID: %s", addon->GetID().c_str());
                    }
                    if (!addon->GetVersion().empty()) {
                        ImGui::Text("Version: %s", addon->GetVersion().c_str());
                    }
                    if (!addon->GetAuthor().empty()) {
                        ImGui::Text("Author: %s", addon->GetAuthor().c_str());
                    }
                    if (!addon->GetFileName().empty()) {
                        ImGui::Text("File: %s", addon->GetFileName().c_str());
                    }
                    if (!addon->GetTypeString().empty()) {
                        ImGui::Text("Type: %s", addon->GetTypeString().c_str());
                    }
                }
                ImGui::EndChild();

                ImGui::BeginChild("##AddonButtons");
                {
                    if (!addon->IsForced() && addon->SupportsLoading()) {
                        // Activate / deactivate button
                        if (addon->GetState() == AddonState::LoadedState) {
                            if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Deactivate", ImVec2(100, 0))) {
                                ADDONS_LOG()->info("Unloading add-on {0}", addon->GetFileName());
                                AppConfig.SetAddonEnabled(addon.get(), false);
                                if (addon->SupportsHotLoading()) {
                                    addon->Unload();
                                }
                                else {
                                    addon->UnloadNextRestart();
                                    ADDONS_LOG()->info("Add-on {0} will be unloaded with next restart", addon->GetFileName());
                                }
                            }
                        }
                        else if (addon->GetState() == AddonState::UnloadedState) {
                            if (ImGui::Button(ICON_MD_POWER_SETTINGS_NEW " Activate", ImVec2(100, 0))) {
                                AppConfig.SetAddonEnabled(addon.get(), false);
                                ADDONS_LOG()->info("Loading add-on {0}", addon->GetFileName());
                                if (addon->SupportsHotLoading()) {
                                    if (addon->Load()) {
                                        auto state = addon->GetState();
                                        if (state == AddonState::LoadedState) {
                                            AppConfig.SetAddonEnabled(addon.get(), true);
                                        }
                                    }
                                    else {
                                        addon->Unload();
                                        ADDONS_LOG()->error("Add-on {0} failed to load", addon->GetFileName());
                                    }
                                }
                                else {
                                    if (addon->LoadNextRestart()) {
                                        AppConfig.SetAddonEnabled(addon.get(), true);
                                        ADDONS_LOG()->info("Add-on {0} will be loaded with next restart", addon->GetFileName());
                                    }
                                }
                            }
                        }
                    }

                    // Settings button
                    if (addon->IsLoaded() && addon->SupportsSettings()) {
                        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 192 - style.ItemSpacing.x - 8); // -8 for the resize grip
                        if (ImGui::Button(ICON_MD_SETTINGS " Settings", ImVec2(96, 0))) {
                            addon->OpenSettings();
                        }
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
            ImGui::TextUnformatted("No add-on selected. Select an add-on in the list to the left.");
            ImGui::PopTextWrapPos();
        }
        else {
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted("No add-ons available. Install at least one add-on and restart Guild Wars 2.");
            ImGui::PopTextWrapPos();
        }
    }

    void SettingsWindow::RenderTabInfo() {
        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(R"(Guild Wars 2 Add-on Loader is an unofficial add-on for Guild Wars 2 developed by Archomeda. It acts as a master add-on that makes it easy to manage your Guild Wars 2 add-ons without the need to sort the files manually.

Addons have to be compatible before they can be used with the Add-on Loader. While incompatible add-ons will still show up in the list of add-ons, you can't activate them. If you want to use an add-on that's not compatible, ask its developer to update it.

Both this add-on and its author do not support add-ons that are against the Guild Wars 2 Terms of Service. Support will not be provided if you're using these kind of add-ons. Please do not use them.

Visit the GitHub repository for more information, third party licenses, and report any issues related to the Add-on Loader there. Keep in mind that the Add-on Loader has no influence on what other add-ons do. Therefore if there are issues related to a specific add-on, please report that issue to its developer instead.


Disclaimer:
The author of this library is not associated with ArenaNet nor with any of its partners. Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners. By using this software, you agree that it is at your own risk and that you assume all responsibility. There is no warranty for using this software.

Also, the author of this library is not responsible for the potential risks by using any additional add-on in combination with this software. It's your responsibility that you use your own judgment to decide whether or not an add-on is allowed by the Guild Wars 2 Terms of Service.
)");
        ImGui::PopTextWrapPos();

        ImGui::Dummy(ImVec2(0, 16));
        if (ImGui::Button(ICON_OC_MARK_GITHUB "  " ICON_OC_LOGO_GITHUB, ImVec2(90, 32))) {
            ShellExecute(0, 0, L"https://github.com/Archomeda/gw2-addon-loader", 0, 0, SW_SHOW);
        }
    }

    void SettingsWindow::RenderTabSettings() {
        ImGui::BeginChild("##Settings", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Add-on Loader Window keybind");
            ImGui::SameLine();
            if (elements::KeybindBox("##LoaderKeybind", this->windowKeybind, &this->windowKeybindEditActive, ImVec2(200, 0))) {
                AppConfig.SetSettingsKeybind(this->windowKeybind);
            }

            if (ImGui::Checkbox("Enable OBS compatibility mode", &this->obsCompatibilityMode)) {
                AppConfig.SetOBSCompatibilityMode(this->obsCompatibilityMode);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(R"(When OBS compatibility mode is active, the add-on loader will
attempt to hide native add-ons and itself from OBS if the
OBS third-party overlays capture setting is disabled.)");
            }

            if (ImGui::Checkbox("Show hidden add-ons", &this->showHiddenAddons)) {
                AppConfig.SetShowHiddenAddons(this->showHiddenAddons);
            }

            if (ImGui::Checkbox("Enable diagnostics", &this->diagnostics)) {
                AppConfig.SetDiagnostics(this->diagnostics);

                // Based on this setting, enable or disable the ThreadMonitor
                if (this->diagnostics) {
                    diagnostics::ThreadMonitor::GetInstance().Start();
                }
                else {
                    diagnostics::ThreadMonitor::GetInstance().Stop();
                }
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Shared API key");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(R"(You can create an API key at account.arena.net.
The API key will be automatically shared to all active add-ons.)");
            }
            ImGui::SameLine();
            if (ImGui::InputTextEx("##ApiKey", this->apiKey, sizeof(this->apiKey), ImVec2(438, 0), ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiTextEditCallbackData *data) {
                ImWchar c = data->EventChar;
                return c >= 0x30 && c <= 0x39 || c >= 0x41 && c <= 0x46 || c == 0x2D ? 0 : 1;
            })) {
                AppConfig.SetApiKey(this->apiKey);
            }
        }
        ImGui::EndChild();

        ImGui::BeginChild("##Metadata");
        {
            ImGui::Text("Add-on Loader version: " VERSION);
        }
        ImGui::EndChild();
    }

    void SettingsWindow::RenderTabDiagnostics() {
        shared_ptr<Addon> selectedAddon;
        stringstream sstream;
        sstream << "Guild Wars 2" << '\0' << "Add-on Loader" << '\0';
        int i = 2;
        for (const auto& addon : Addons) {
            if ((AppConfig.GetShowHiddenAddons() || !addon->IsHidden()) && addon->GetState() == AddonState::LoadedState) {
                sstream << addon->GetName() << '\0';
                if (this->selectedStatsType == i) {
                    selectedAddon = addon;
                }
                ++i;
            }
        }

        ImGui::PushItemWidth(-1);
        const string types = sstream.str();
        ImGui::Combo("##RenderType", &this->selectedStatsType, types.c_str());

        if (this->selectedStatsType == 0) {
            ImGui::PlotLines("##RenderingTime", &hooks::DurationHistoryD3D9Processing[0], static_cast<int>(hooks::DurationHistoryD3D9Processing.size()), 0, "Frame render time (ms)", 0, 67, ImVec2(0, 120));

            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGuiStyle& style = ImGui::GetStyle();
            ImRect plotRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            float yFps30 = plotRect.Max.y - (plotRect.GetSize().y / 2.0f);
            float yFps60 = plotRect.Max.y - (plotRect.GetSize().y / 4.0f);
            draw->AddLine(ImVec2(plotRect.Min.x, yFps30), ImVec2(plotRect.Max.x, yFps30), IM_COL32(192, 144, 96, 255));
            draw->AddText(ImVec2(plotRect.Min.x + 2, yFps30 + 1), IM_COL32(192, 144, 96, 255), "30fps");
            draw->AddLine(ImVec2(plotRect.Min.x, yFps60), ImVec2(plotRect.Max.x, yFps60), IM_COL32(96, 192, 96, 255));
            draw->AddText(ImVec2(plotRect.Min.x + 2, yFps60 + 1), IM_COL32(96, 192, 96, 255), "60fps");
            ImGui::Spacing();

            const auto& link = hooks::MumbleLink::GetInstance();
            if (ImGui::CollapsingHeader("Mumble Link")) {
                if (link.GetBuildId() > 0) {
                    ImGui::BeginColumns("##MumbleLink", 2, ImGuiColumnsFlags_::ImGuiColumnsFlags_NoBorder | ImGuiColumnsFlags_::ImGuiColumnsFlags_NoResize);
                    {
                        elements::SetColumnWidth(0, 150);
                        ImGui::TextUnformatted("Build ID");
                        ImGui::NextColumn();
                        ImGui::Text("%d", link.GetBuildId());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Character Name");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted(link.GetCharacterName().c_str());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Profession");
                        ImGui::NextColumn();
                        ImGui::Text("%s (%d)", hooks::ProfessionToString(link.GetProfession()).c_str(), link.GetProfession());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Race");
                        ImGui::NextColumn();
                        ImGui::Text("%s (%d)", hooks::RaceToString(link.GetRace()).c_str(), link.GetRace());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Commander?");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted(link.IsCommander() ? "Yes" : "No");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Map ID");
                        ImGui::NextColumn();
                        ImGui::Text("%d", link.GetMapId());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Map Type");
                        ImGui::NextColumn();
                        ImGui::Text("%s (%d)", hooks::MapTypeToString(link.GetMapType()).c_str(), link.GetMapType());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Competitive Map?");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted(link.IsTypeCompetitive() ? "Yes" : "No");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Shard ID");
                        ImGui::NextColumn();
                        ImGui::Text("%d", link.GetShardId());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Server Address");
                        ImGui::NextColumn();
                        ImGui::Text("%d.%d.%d.%d", link.GetServerAddress().sin_addr.S_un.S_un_b.s_b1, link.GetServerAddress().sin_addr.S_un.S_un_b.s_b2, link.GetServerAddress().sin_addr.S_un.S_un_b.s_b3, link.GetServerAddress().sin_addr.S_un.S_un_b.s_b4);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Team Color");
                        ImGui::NextColumn();
                        ImGui::Text("%d", link.GetTeamColorId());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("FoV");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f", link.GetFov());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("UI Size");
                        ImGui::NextColumn();
                        ImGui::Text("%s (%d)", hooks::UiSizeToString(link.GetUiSize()).c_str(), link.GetUiSize());
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Character Position");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCharacterPosition().x, link.GetCharacterPosition().y, link.GetCharacterPosition().z);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Character Top");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCharacterTop().x, link.GetCharacterTop().y, link.GetCharacterTop().z);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Character Front");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCharacterFront().x, link.GetCharacterFront().y, link.GetCharacterFront().z);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Camera Position");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCameraPosition().x, link.GetCameraPosition().y, link.GetCameraPosition().z);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Camera Top");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCameraTop().x, link.GetCameraTop().y, link.GetCameraTop().z);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Camera Front");
                        ImGui::NextColumn();
                        ImGui::Text("%.4f, %.4f, %.4f", link.GetCameraFront().x, link.GetCameraFront().y, link.GetCameraFront().z);
                        ImGui::NextColumn();
                    }
                    ImGui::EndColumns();
                }
            }
            if (ImGui::CollapsingHeader("Threads")) {
                ImGui::BeginColumns("##Threads", 6, 0);
                {
                    elements::SetColumnWidth(0, 50);
                    elements::SetColumnWidth(1, 90);
                    elements::SetColumnWidth(2, 120);
                    elements::SetColumnWidth(3, 50);
                    elements::SetColumnWidth(4, 50);
                    ImGui::TextUnformatted("ID");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Process");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Name");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Kernel");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("User");
                    ImGui::NextColumn();
                    ImGui::TextUnformatted("Priority");
                    ImGui::NextColumn();
                    for (const auto& thread : ThreadMonitor::GetInstance().GetThreadInfos()) {
                        ImGui::Text("%d", thread->threadId);
                        ImGui::NextColumn();
                        ImGui::Text("%s (%d)", thread->processName.c_str(), thread->processId);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted(!thread->threadDescription.empty() ? thread->threadDescription.c_str() : thread->threadName.c_str());
                        ImGui::NextColumn();
                        if (thread->kernelTimePercentage >= 0.1) {
                            ImGui::Text("%.1f%%", thread->kernelTimePercentage);
                        }
                        ImGui::NextColumn();
                        if (thread->userTimePercentage >= 0.1) {
                            ImGui::Text("%.1f%%", thread->userTimePercentage);
                        }
                        ImGui::NextColumn();
                        ImGui::Text("%s - %s", thread->GetProcessPriorityString().c_str(), thread->GetBasePriorityString().c_str());
                        ImGui::NextColumn();
                    }
                }
                ImGui::EndColumns();
            }
        }
        else if (this->selectedStatsType == 1) {
            ImGui::PlotLines("##RenderingTime", &hooks::DurationHistoryLoaderDrawFrame[0], static_cast<int>(hooks::DurationHistoryLoaderDrawFrame.size()), 0, "Frame render time (ms)", 0, 4, ImVec2(0, 70));
        }
        else if (selectedAddon) {
            if (ImGui::CollapsingHeader("General")) {
                ImGui::BeginColumns("##AddonGeneral", 5, 0);
                {
                    elements::SetColumnWidth(0, 150);
                    elements::SetColumnWidth(1, 110);
                    elements::SetColumnWidth(2, 110);
                    elements::SetColumnWidth(3, 110);
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
                    const auto& metric = selectedAddon->GetMetricLoad();
                    ImGui::TextUnformatted(metric.FormatTimeMetric(metric.GetLast()).c_str());
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    if (selectedAddon->HandleWndProc) this->ImGuiAddonStatLine("WndProc", selectedAddon->HandleWndProc.GetMetric());
                }
                ImGui::EndColumns();
            }
            if (selectedAddon->HasRenderingHooks()) {
                if (ImGui::CollapsingHeader("Rendering")) {
                    const auto history = selectedAddon->GetMetricOverall().GetMovingHistory();
                    ImGui::PlotLines("##RenderingTime", &history[0], static_cast<int>(history.size()), 0, "Add-on frame time (µs)", 0, 10000, ImVec2(0, 100));
                    ImGui::BeginColumns("##AddonRenderingTime", 5, 0);
                    {
                        elements::SetColumnWidth(0, 150);
                        elements::SetColumnWidth(1, 110);
                        elements::SetColumnWidth(2, 110);
                        elements::SetColumnWidth(3, 110);
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Current");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Exp. moving avg");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Overall max");
                        ImGui::NextColumn();
                        ImGui::TextUnformatted("Calls");
                        ImGui::NextColumn();

                        this->ImGuiAddonStatLine("Total", selectedAddon->GetMetricOverall(), false);
                        if (selectedAddon->DrawFrameBeforePostProcessing) this->ImGuiAddonStatLine("DrawBeforePostProcessing", selectedAddon->DrawFrameBeforePostProcessing.GetMetric());
                        if (selectedAddon->DrawFrameBeforeGui) this->ImGuiAddonStatLine("DrawBeforeGui", selectedAddon->DrawFrameBeforeGui.GetMetric());
                        if (selectedAddon->DrawFrame) this->ImGuiAddonStatLine("Draw", selectedAddon->DrawFrame.GetMetric());
                        if (selectedAddon->AdvPreBeginScene) this->ImGuiAddonStatLine("AdvPreBeginScene", selectedAddon->AdvPreBeginScene.GetMetric());
                        if (selectedAddon->AdvPostBeginScene) this->ImGuiAddonStatLine("AdvPostBeginScene", selectedAddon->AdvPostBeginScene.GetMetric());
                        if (selectedAddon->AdvPreEndScene) this->ImGuiAddonStatLine("AdvPreEndScene", selectedAddon->AdvPreEndScene.GetMetric());
                        if (selectedAddon->AdvPostEndScene) this->ImGuiAddonStatLine("AdvPostEndScene", selectedAddon->AdvPostEndScene.GetMetric());
                        if (selectedAddon->AdvPreClear) this->ImGuiAddonStatLine("AdvPreClear", selectedAddon->AdvPreClear.GetMetric());
                        if (selectedAddon->AdvPostClear) this->ImGuiAddonStatLine("AdvPostClear", selectedAddon->AdvPostClear.GetMetric());
                        if (selectedAddon->AdvPreReset) this->ImGuiAddonStatLine("AdvPreReset", selectedAddon->AdvPreReset.GetMetric());
                        if (selectedAddon->AdvPostReset) this->ImGuiAddonStatLine("AdvPostReset", selectedAddon->AdvPostReset.GetMetric());
                        if (selectedAddon->AdvPrePresent) this->ImGuiAddonStatLine("AdvPrePresent", selectedAddon->AdvPrePresent.GetMetric());
                        if (selectedAddon->AdvPostPresent) this->ImGuiAddonStatLine("AdvPostPresent", selectedAddon->AdvPostPresent.GetMetric());
                        if (selectedAddon->AdvPreCreateTexture) this->ImGuiAddonStatLine("AdvPreCreateTexture", selectedAddon->AdvPreCreateTexture.GetMetric());
                        if (selectedAddon->AdvPostCreateTexture) this->ImGuiAddonStatLine("AdvPostCreateTexture", selectedAddon->AdvPostCreateTexture.GetMetric());
                        if (selectedAddon->AdvPreCreateVertexShader) this->ImGuiAddonStatLine("AdvPreCreateVertexShader", selectedAddon->AdvPreCreateVertexShader.GetMetric());
                        if (selectedAddon->AdvPostCreateVertexShader) this->ImGuiAddonStatLine("AdvPostCreateVertexShader", selectedAddon->AdvPostCreateVertexShader.GetMetric());
                        if (selectedAddon->AdvPreCreatePixelShader) this->ImGuiAddonStatLine("AdvPreCreatePixelShader", selectedAddon->AdvPreCreatePixelShader.GetMetric());
                        if (selectedAddon->AdvPostCreatePixelShader) this->ImGuiAddonStatLine("AdvPostCreatePixelShader", selectedAddon->AdvPostCreatePixelShader.GetMetric());
                        if (selectedAddon->AdvPreCreateRenderTarget) this->ImGuiAddonStatLine("AdvPreCreateRenderTarget", selectedAddon->AdvPreCreateRenderTarget.GetMetric());
                        if (selectedAddon->AdvPostCreateRenderTarget) this->ImGuiAddonStatLine("AdvPostCreateRenderTarget", selectedAddon->AdvPostCreateRenderTarget.GetMetric());
                        if (selectedAddon->AdvPreSetTexture) this->ImGuiAddonStatLine("AdvPreSetTexture", selectedAddon->AdvPreSetTexture.GetMetric());
                        if (selectedAddon->AdvPostSetTexture) this->ImGuiAddonStatLine("AdvPostSetTexture", selectedAddon->AdvPostSetTexture.GetMetric());
                        if (selectedAddon->AdvPreSetVertexShader) this->ImGuiAddonStatLine("AdvPreSetVertexShader", selectedAddon->AdvPreSetVertexShader.GetMetric());
                        if (selectedAddon->AdvPostSetVertexShader) this->ImGuiAddonStatLine("AdvPostSetVertexShader", selectedAddon->AdvPostSetVertexShader.GetMetric());
                        if (selectedAddon->AdvPreSetPixelShader) this->ImGuiAddonStatLine("AdvPreSetPixelShader", selectedAddon->AdvPreSetPixelShader.GetMetric());
                        if (selectedAddon->AdvPostSetPixelShader) this->ImGuiAddonStatLine("AdvPostSetPixelShader", selectedAddon->AdvPostSetPixelShader.GetMetric());
                        if (selectedAddon->AdvPreSetRenderTarget) this->ImGuiAddonStatLine("AdvPreSetRenderTarget", selectedAddon->AdvPreSetRenderTarget.GetMetric());
                        if (selectedAddon->AdvPostSetRenderTarget) this->ImGuiAddonStatLine("AdvPostSetRenderTarget", selectedAddon->AdvPostSetRenderTarget.GetMetric());
                        if (selectedAddon->AdvPreSetRenderState) this->ImGuiAddonStatLine("AdvPreSetRenderState", selectedAddon->AdvPreSetRenderState.GetMetric());
                        if (selectedAddon->AdvPostSetRenderState) this->ImGuiAddonStatLine("AdvPostSetRenderState", selectedAddon->AdvPostSetRenderState.GetMetric());
                        if (selectedAddon->AdvPreDrawIndexedPrimitive) this->ImGuiAddonStatLine("AdvPreDrawIndexedPrimitive", selectedAddon->AdvPreDrawIndexedPrimitive.GetMetric());
                        if (selectedAddon->AdvPostDrawIndexedPrimitive) this->ImGuiAddonStatLine("AdvPostDrawIndexedPrimitive", selectedAddon->AdvPostDrawIndexedPrimitive.GetMetric());
                    }
                    ImGui::EndColumns();
                }
            }
        }

        ImGui::PopItemWidth();
    }

    void SettingsWindow::RenderTabUpdate() {
        ImGui::BeginChild("##Update");
        {
            if (this->IsAddonLoaderUpdateAvailable()) {
                ImGui::PushTextWrapPos();
                ImGui::TextUnformatted("A new update for the Add-on Loader is available. Please refer to the release notes to see what has changed. Click the \"Update\" button to automatically update the Add-on Loader. After the update has finished, a full Guild Wars 2 restart is required.");
                ImGui::PopTextWrapPos();
                ImGui::Dummy(ImVec2(0, 4));
                ImGui::Text("Installed version: %s", VERSION);
                ImGui::Text("New version: %s", AppConfig.GetLatestVersion().c_str());
                ImGui::Dummy(ImVec2(0, 4));
                if (ImGui::Button(ICON_OC_MARK_GITHUB " Open release notes", ImVec2(160, 32))) {
                    ShellExecute(0, 0, u16(AppConfig.GetLatestVersionInfoUrl()).c_str(), 0, 0, SW_SHOW);
                }
                ImGui::SameLine();
                if (LoaderUpdaterInstaller != nullptr && (LoaderUpdaterInstaller->IsActive() || LoaderUpdaterInstaller->HasCompleted())) {
                    ImGui::ProgressBar(LoaderUpdaterInstaller->GetProgressFraction(), ImVec2(-1, 32), LoaderUpdaterInstaller->GetDetailedProgress().c_str());
                }
                else {
                    if (ImGui::Button(ICON_MD_FILE_DOWNLOAD " Update", ImVec2(100, 32))) {
                        InstallUpdate();
                    }
                }
            }
        }
        ImGui::EndChild();
    }


    bool SettingsWindow::IsAddonLoaderUpdateAvailable() {
        string latestVersion = AppConfig.GetLatestVersion();
        return !latestVersion.empty() && latestVersion != VERSION;
    }


    void SettingsWindow::SelectAddon(const Addon* const addon) {
        int index = -1;
        for (const auto& addon_ : Addons) {
            if (AppConfig.GetShowHiddenAddons() || !addon_->IsHidden()) {
                index++;
                if (addon_->GetID() == addon->GetID()) {
                    this->selectedAddon = index;
                    break;
                }
            }
        }
    }


    void SettingsWindow::PushTabStyle(int tabIndex) {
        ImGuiStyle& style = ImGui::GetStyle();
        if (this->selectedTab == tabIndex) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, style.Colors[ImGuiCol_Button].w));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        }
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, style.Colors[ImGuiCol_ButtonHovered].w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, style.Colors[ImGuiCol_ButtonActive].w));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.89f, 0.88f, 0.68f, 1.00f));
        ImGui::PushFont(imgui::FontIconButtons);
    }

    void SettingsWindow::PopTabStyle(int tabIndex) {
        ImGui::PopStyleColor(4);
        ImGui::PopFont();
    }

}
