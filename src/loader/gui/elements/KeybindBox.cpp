#include "KeybindBox.h"
#include "../../input.h"

using namespace std;

namespace loader {
    namespace gui {
        namespace elements {

            bool KeybindBox(const char* label, set<uint_fast8_t>& keys, bool* selected, const ImVec2& size) {
                const set<uint_fast8_t> pressedKeys = GetPressedKeyboardKeys();
                const string keysStr = GetReadableKeyString(*selected ? pressedKeys : keys);
                char keysBuff[64];
                keysStr._Copy_s(keysBuff, sizeof(keysBuff), keysStr.length());
                keysBuff[keysStr.length()] = 0;

                ImGui::InputTextEx(label, keysBuff, sizeof(keysBuff), size, ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemActive()) {
                    *selected = true;
                    if (DoKeysContainNonModifiers(pressedKeys)) {
                        // Apply keybind
                        ImGui::ClearActiveID();
                        keys = pressedKeys;
                        *selected = false;
                        return true;
                    }
                }
                else if (*selected) {
                    // Reset keybind
                    *selected = false;
                }
                else if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to activate the field and press a new keybind. Use Escape to cancel.");
                }
                return false;
            }

        }
    }
}
