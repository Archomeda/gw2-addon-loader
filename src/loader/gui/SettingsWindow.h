#pragma once
#include "Window.h"
#include <memory>
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
            int selectedAddon = -1;
            int selectedTab = 0;
            int selectedStatsType = 0;
            bool statsDrawFrame = true;
            bool statsDrawFrameBeforeGui = false;
            bool statsDrawFrameBeforePostProcessing = false;

            bool initializedState = false;
            bool showUnsupportedAddons;
            bool windowKeybindEditActive = false;
            std::set<uint_fast8_t> windowKeybind;
            bool showDebugFeatures;

            bool ImGuiAddonsList(const char* label, int* current_item, const std::vector<std::shared_ptr<addons::Addon>> addons, const ImVec2& listBoxSize, float listItemHeight);
            void RenderTabAddons();
            void RenderTabInfo();
            void RenderTabSettings();
            void RenderTabStats();

            void MoveAddonPositionUp(const std::string& fileName);
            void MoveAddonPositionDown(const std::string& fileName);
            void SelectAddon(const std::string& fileName);

            void PushTabStyle(int tabIndex);
            void PopTabStyle(int tabIndex);
        };
    
    }
}
