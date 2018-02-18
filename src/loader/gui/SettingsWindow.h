#pragma once
#include "Window.h"
#include <string>

namespace loader {
    namespace gui {

        class SettingsWindow : public Window {
        public:
            SettingsWindow();

            void Render() override;

        protected:
            int selectedAddon = -1;

            void MoveAddonPositionUp(const std::wstring& fileName);
            void MoveAddonPositionDown(const std::wstring& fileName);
            void SelectAddon(const std::wstring& fileName);
        };
    
    }
}
