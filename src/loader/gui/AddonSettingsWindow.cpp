#include "AddonSettingsWindow.h"
#include "gui_manager.h"
#include "elements/ExtraImGuiElements.h"
#include "../addons/NativeAddon.h"
#include "../log.h"

#define FLOATCOLTOINT(f) (static_cast<int>(f * 255.0f + 0.5f))

using namespace std;
using namespace loader::addons;
using namespace loader::gui::elements;

namespace loader::gui {

    AddonSettingsWindow::AddonSettingsWindow(shared_ptr<Addon> addon) : addon(addon) {
        this->SetTitle(addon->GetName() + " Settings");
        this->SetFlags(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    void AddonSettingsWindow::Render() {
        for (size_t i = 0; i < this->definitions.size(); ++i) {
            const auto& definition = this->definitions[i];
            auto& value = this->values[i];
            void* valuePointer = this->valuePointers[i];

            switch (definition.type) {
            case AddonSettingsEntryType::SettingsTypeSeparator:
                ImGui::Separator();
                break;
            case AddonSettingsEntryType::SettingsTypeText:
                ImGui::PushTextWrapPos();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::PopTextWrapPos();
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeBoolean:
                ImGui::Checkbox(definition.name.c_str(), &value.boolean);
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeInteger:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                ImGui::InputInt(("##" + definition.name).c_str(), &value.integer, 0, 0);
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeString:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                ImGui::InputText(("##" + definition.name).c_str(), value.string, definition.valueSize);
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeOption:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                if (ImGui::BeginCombo(("##" + definition.name).c_str(), definition.listNames[value.option].c_str())) {
                    if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(definition.hint.c_str());
                    }
                    for (size_t j = 0; j < definition.listNames.size(); ++j) {
                        bool selected = value.option == j;
                        if (ImGui::Selectable(definition.listNames[j].c_str(), selected)) {
                            value.option = static_cast<int>(j);
                        }
                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        if (!definition.listHints[j].empty() && ImGui::IsItemHovered()) {
                            ImGui::SetTooltip(definition.listHints[j].c_str());
                        }
                    }
                    ImGui::EndCombo();
                }
                else {
                    if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(definition.hint.c_str());
                    }
                }
                break;
            case AddonSettingsEntryType::SettingsTypeKeybind:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                if (definition.valueSize < 4) {
                    // Just use one key
                    elements::KeybindBox(("## " + definition.name).c_str(), &value.keybind, &value.keybindBoxActive, ImVec2(200, 0));
                }
                else {
                    // Use all keys
                    elements::KeybindBox(("## " + definition.name).c_str(), value.keybindSet, &value.keybindBoxActive, ImVec2(200, 0));
                }
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeRgb:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                ImGui::ColorEdit3(("##" + definition.name).c_str(), value.colors);
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            case AddonSettingsEntryType::SettingsTypeArgb:
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(definition.name.c_str());
                ImGui::SameLine();
                ImGui::ColorEdit4(("##" + definition.name).c_str(), value.colors);
                if (!definition.hint.empty() && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(definition.hint.c_str());
                }
                break;
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(200, 0))) {
            for (size_t i = 0; i < this->definitions.size(); ++i) {
                const auto& definition = this->definitions[i];
                const auto& value = this->values[i];
                void* valuePointer = this->valuePointers[i];
                switch (definition.type) {
                case AddonSettingsEntryType::SettingsTypeBoolean: {
                    bool* boolPtr = static_cast<bool*>(valuePointer);
                    *boolPtr = value.boolean;
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeInteger: {
                    int* intPtr = static_cast<int*>(valuePointer);
                    *intPtr = value.integer;
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeString: {
                    char* charPtr = static_cast<char*>(valuePointer);
                    memcpy_s(charPtr, sizeof(char) * definition.valueSize, value.string, sizeof(char) * definition.valueSize);
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeOption: {
                    int* optionPtr = static_cast<int*>(valuePointer);
                    *optionPtr = value.option;
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeKeybind: {
                    int* keybindPtr = static_cast<int*>(valuePointer);
                    memset(keybindPtr, 0, sizeof(int) * definition.valueSize);
                    if (definition.valueSize < 4) {
                        // Just use one key
                        keybindPtr[0] = value.keybind;
                    }
                    else {
                        // Use all keys
                        size_t i = 0;
                        for (auto key : value.keybindSet) {
                            if (i >= static_cast<size_t>(definition.valueSize)) {
                                // Prevent out-of-bounds crash
                                break;
                            }
                            keybindPtr[i] = key;
                            ++i;
                        }
                    }
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeRgb: {
                    int* intPtr = static_cast<int*>(valuePointer);
                    *intPtr = (FLOATCOLTOINT(value.colors[0]) << 16) | (FLOATCOLTOINT(value.colors[1]) << 8) | FLOATCOLTOINT(value.colors[2]);
                    break;
                }
                case AddonSettingsEntryType::SettingsTypeArgb: {
                    int* intPtr = static_cast<int*>(valuePointer);
                    *intPtr = (FLOATCOLTOINT(value.colors[0]) << 16) | (FLOATCOLTOINT(value.colors[1]) << 8) | FLOATCOLTOINT(value.colors[2]) | (FLOATCOLTOINT(value.colors[3]) << 24);
                    break;
                }
                }
            }
            this->Close();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(200, 0))) {
            this->Close();
        }
    }

    void AddonSettingsWindow::OnClose() {
        this->addon->CloseSettings();
    }

    void AddonSettingsWindow::AddSetting(AddonSettingsEntry setting) {
        if (setting.value == nullptr &&
            (setting.definition.type != AddonSettingsEntryType::SettingsTypeSeparator && setting.definition.type != AddonSettingsEntryType::SettingsTypeText)) {
            GUI_LOG()->error("Setting {0} has an invalid value: NULL, skipping...", setting.definition.name);
            return;
        }

        AddonSettingsWindowDefinition definition;
        definition.type = setting.definition.type;
        if (setting.definition.name != nullptr) {
            definition.name = setting.definition.name;
        }
        if (setting.definition.hint != nullptr) {
            definition.hint = setting.definition.hint;
        }
        for (int i = 0; i < setting.definition.listSize; ++i) {
            definition.listNames.push_back(setting.definition.listNames[i]);
            if (setting.definition.listHints != nullptr && setting.definition.listHints[i] != nullptr) {
                definition.listHints.push_back(setting.definition.listHints[i]);
            }
            else {
                definition.listHints.push_back(string());
            }
        }
        definition.valueSize = setting.definition.valueSize;
        this->definitions.push_back(definition);

        AddonSettingsWindowValue value;
        switch (setting.definition.type) {
        case AddonSettingsEntryType::SettingsTypeBoolean:
            value.boolean = *setting.boolValue;
            break;
        case AddonSettingsEntryType::SettingsTypeInteger:
            value.integer = *setting.intValue;
            break;
        case AddonSettingsEntryType::SettingsTypeString:
            value.string = new char[setting.definition.valueSize];
            memcpy_s(value.string, sizeof(char) * setting.definition.valueSize, setting.stringValue, sizeof(char) * setting.definition.valueSize);
            break;
        case AddonSettingsEntryType::SettingsTypeOption:
            value.option = *setting.optionValue;
            break;
        case AddonSettingsEntryType::SettingsTypeKeybind:
            value.keybind = setting.keybindValue[0];
            for (int i = 0; i < setting.definition.listSize; ++i) {
                if (setting.keybindValue[i] != 0) {
                    value.keybindSet.insert(setting.keybindValue[i]);
                }
            }
            break;
        case AddonSettingsEntryType::SettingsTypeRgb:
            value.colors[0] = static_cast<float>((*setting.rgbValue >> 16) & 0xFF) / 255.0f;
            value.colors[1] = static_cast<float>((*setting.rgbValue >> 8) & 0xFF) / 255.0f;
            value.colors[2] = static_cast<float>(*setting.rgbValue & 0xFF) / 255.0f;
            break;
        case AddonSettingsEntryType::SettingsTypeArgb:
            value.colors[0] = static_cast<float>((*setting.rgbValue >> 16) & 0xFF) / 255.0f;
            value.colors[1] = static_cast<float>((*setting.rgbValue >> 8) & 0xFF) / 255.0f;
            value.colors[2] = static_cast<float>(*setting.rgbValue & 0xFF) / 255.0f;
            value.colors[3] = static_cast<float>((*setting.rgbValue >> 24) & 0xFF) / 255.0f;
            break;
        }
        this->values.push_back(value);
        this->valuePointers.push_back(setting.value);
    }

}
