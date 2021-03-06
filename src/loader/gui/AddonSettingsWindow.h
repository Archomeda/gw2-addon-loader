#pragma once
#include "../stdafx.h"
#include "Window.h"

namespace loader::addons {
    class Addon;
}

namespace loader::gui {

    struct AddonSettingsWindowDefinition {
        AddonSettingsEntryType type;
        std::string name;
        std::string hint;
        std::vector<std::string> listNames;
        std::vector<std::string> listHints;
        int valueSize;
    };

    struct AddonSettingsWindowValue {
        union {
            bool boolean;
            int integer;
            char* string;
            int option;
        };

        uint_fast8_t keybind;
        std::set<uint_fast8_t> keybindSet;
        bool keybindBoxActive = false;
        float colors[4];
    };

    class AddonSettingsWindow : public Window {
    public:
        AddonSettingsWindow(std::shared_ptr<addons::Addon> addon);

        virtual void Render() override;
        virtual void OnClose() override;

        void AddSetting(AddonSettingsEntry setting);

    private:
        std::shared_ptr<addons::Addon> addon;
        std::vector<AddonSettingsWindowDefinition> definitions;
        std::vector<void*> valuePointers;
        std::vector<AddonSettingsWindowValue> values;
    };

}
