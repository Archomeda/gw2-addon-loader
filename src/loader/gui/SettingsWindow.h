#pragma once
#include "Window.h"
#include <set>
#include <vector>
#include "../addons/Addon.h"

namespace loader {
    namespace gui {

        class SettingsWindow : public Window {
        public:
            SettingsWindow();

            void Render() override;

            void BeginStyle() override;
            void EndStyle() override;

        private:
            bool ImGuiAddonsList(const char* label, int* current_item, const std::vector<addons::Addon*> addons, const ImVec2& listBoxSize, float listItemHeight);
            void ImGuiAddonStatLine(const char* label, const addons::AddonMetric& measure, bool calls);

            void RenderTabAddons();
            void RenderTabInfo();
            void RenderTabSettings();
            void RenderTabStats();
            void RenderTabUpdate();

            bool IsAddonLoaderUpdateAvailable();

            void SelectAddon(const addons::Addon* const addon);

            void PushTabStyle(int tabIndex);
            void PopTabStyle(int tabIndex);

            int selectedAddon = -1;
            int selectedTab = 0;
            int selectedStatsType = 0;
            bool statsDrawFrame = true;
            bool statsDrawFrameBeforeGui = false;
            bool statsDrawFrameBeforePostProcessing = false;

            bool initializedState = false;
            bool obsCompatibilityMode;
            bool showIncompatibleAddons;
            bool windowKeybindEditActive = false;
            std::set<uint_fast8_t> windowKeybind;
            bool showDebugFeatures;
        };

    }
}
