#include "input.h"

using namespace std;

namespace loader::imgui {

    set<uint_fast8_t> pressedKeys;
    bool repeatedPressedKeys = false;

    const char vkStrings[256][16] = {
        "M1", "M2", "", "M3", "M4", "M5", "", "", "Backspace", "Tab", "", "", "Clear", "Enter", "", "",
        "Shift", "Ctr", "Alt", "Pause", "Caps Lock", "", "", "", "", "", "", "Escape", "", "", "", "",
        "Space", "Page Up", "Page Down", "End", "Home", "Left", "Up", "Right", "Down", "Select", "", "", "Print Screen", "Insert", "Delete", "Help",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "",
        "", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
        "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Win", "Right Win", "", "", "Sleep",
        "NUM 0", "NUM 1", "NUM 2", "NUM 3", "NUM 4", "NUM 5", "NUM 6", "NUM 7", "NUM 8", "NUM 9", "NUM *", "NUM +", "", "NUM -", "NUM .", "NUM /",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
        "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
        "Num Lock", "Scroll Lock"
    };

    void ProcessInputMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_KILLFOCUS) {
            // Lost focus, reset our pressed keys
            pressedKeys.clear();
            return;
        }

        struct EventKey {
            uint_fast8_t vk;
            bool down;
        };

        vector<EventKey> newKeys;

        // Generate our key list for the current message
        {
            bool isDown = false;
            switch (msg) {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                isDown = true;
            case WM_SYSKEYUP:
            case WM_KEYUP:
                if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP)
                {
                    if (HIWORD(lParam) & KF_ALTDOWN) {
                        newKeys.push_back({ VK_MENU, true });
                    }
                    else {
                        newKeys.push_back({ VK_MENU, false });
                    }
                }
                newKeys.push_back({ static_cast<uint_fast8_t>(wParam), isDown });
                break;
            case WM_LBUTTONDOWN:
                isDown = true;
            case WM_LBUTTONUP:
                newKeys.push_back({ VK_LBUTTON, isDown });
                break;
            case WM_MBUTTONDOWN:
                isDown = true;
            case WM_MBUTTONUP:
                newKeys.push_back({ VK_MBUTTON, isDown });
                break;
            case WM_RBUTTONDOWN:
                isDown = true;
            case WM_RBUTTONUP:
                newKeys.push_back({ VK_RBUTTON, isDown });
                break;
            case WM_XBUTTONDOWN:
                isDown = true;
            case WM_XBUTTONUP:
                newKeys.push_back({ (uint_fast8_t)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), isDown });
                break;
            }
        }

        // Process the differences
        for (auto k : newKeys) {
            if (k.down) {
                pressedKeys.insert(k.vk);
            }
            else {
                pressedKeys.erase(k.vk);
            }
        }
        repeatedPressedKeys = newKeys.empty();
    }

    bool RepeatedPressedKeys() {
        return repeatedPressedKeys;
    }

    const set<uint_fast8_t> GetPressedKeys() {
        return set<uint_fast8_t>(pressedKeys);
    }

    const set<uint_fast8_t> GetPressedKeyboardKeys() {
        set<uint_fast8_t> keys;
        copy_if(pressedKeys.begin(), pressedKeys.end(), inserter(keys, keys.end()), [](const uint_fast8_t k) {
            switch (k) {
            case VK_LBUTTON:
            case VK_RBUTTON:
            case VK_MBUTTON:
            case VK_XBUTTON1:
            case VK_XBUTTON2:
                return false;
            }
            return true;
        });
        return keys;
    }

    const set<uint_fast8_t> GetPressedMouseKeys() {
        set<uint_fast8_t> keys;
        copy_if(pressedKeys.begin(), pressedKeys.end(), inserter(keys, keys.end()), [](const uint_fast8_t k) {
            switch (k) {
            case VK_LBUTTON:
            case VK_RBUTTON:
            case VK_MBUTTON:
            case VK_XBUTTON1:
            case VK_XBUTTON2:
                return true;
            }
            return false;
        });
        return keys;
    }

    bool DoKeysContainNonModifiers(const set<uint_fast8_t>& keys) {
        for (auto k : keys) {
            switch (k) {
            case VK_CONTROL:
            case VK_MENU:
            case VK_SHIFT:
                break;
            default:
                return true;
            }
        }
        return false;
    }

    const string GetReadableKeyString(const set<uint_fast8_t>& keys) {
        set<uint_fast8_t> remaining(keys);
        stringstream ss;
        if (remaining.find(VK_CONTROL) != remaining.end()) {
            ss << "Ctrl + ";
            remaining.erase(VK_CONTROL);
        }
        if (remaining.find(VK_MENU) != remaining.end()) {
            ss << "Alt + ";
            remaining.erase(VK_MENU);
        }
        if (remaining.find(VK_SHIFT) != remaining.end()) {
            ss << "Shift + ";
            remaining.erase(VK_SHIFT);
        }
        bool doneFirst = false;
        for (const auto k : remaining) {
            auto str = vkStrings[k];
            if (!str[0]) {
                if (doneFirst) {
                    ss << " + ";
                }

                // Empty key, try getting the actual translation from Windows
                UINT code = MapVirtualKey(k, MAPVK_VK_TO_VSC);
                if (code) {
                    char chr[8];
                    GetKeyNameTextA(code << 16, chr, 8);
                    ss << chr;
                }
            }
            else {
                if (doneFirst) {
                    ss << " + ";
                }
                ss << str;
            }
            doneFirst = true;
        }

        return ss.str();
    }

}
