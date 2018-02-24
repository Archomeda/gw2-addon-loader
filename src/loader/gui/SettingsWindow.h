#pragma once
#include "Window.h"
#include <string>

namespace loader {
    namespace gui {

        class SettingsWindow : public Window {
        public:
            SettingsWindow();

            void Render() override;

            void BeginStyle() override;
            void EndStyle() override;

        protected:
            int selectedAddon = -1;
            int selectedTab = 0;

            bool ImGuiAddonsList(const char* label, int* current_item, const char* const* items, int items_count, const ImVec2& listBoxSize, float listItemHeight);
            void RenderTabAddons();
            void RenderTabInfo();
            void RenderTabSettings();
            void RenderTabStats();

            void MoveAddonPositionUp(const std::wstring& fileName);
            void MoveAddonPositionDown(const std::wstring& fileName);
            void SelectAddon(const std::wstring& fileName);

            void PushTabStyle(int tabIndex);
            void PopTabStyle(int tabIndex);
        };
    
    }
}
