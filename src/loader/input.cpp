#include "input.h"
#include "windows.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;

namespace loader {

    set<uint_fast8_t> pressedKeys;
    bool repeatedPressedKeys = false;

    const wchar_t vkStrings[256][16] = {
        L"M1", L"M2", L"", L"M3", L"M4", L"M5", L"", L"", L"Backspace", L"Tab", L"", L"", L"Clear", L"Enter", L"", L"",
        L"Shift", L"Ctrl", L"Alt", L"Pause", L"Caps Lock", L"", L"", L"", L"", L"", L"", L"Escape", L"", L"", L"", L"",
        L"Space", L"Page Up", L"Page Down", L"End", L"Home", L"Left", L"Up", L"Right", L"Down", L"Select", L"", L"", L"Print Screen", L"Insert", L"Delete", L"Help",
        L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"", L"", L"", L"", L"", L"",
        L"", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O",
        L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", L"Z", L"Left Win", L"Right Win", L"", L"", L"Sleep",
        L"NUM 0", L"NUM 1", L"NUM 2", L"NUM 3", L"NUM 4", L"NUM 5", L"NUM 6", L"NUM 7", L"NUM 8", L"NUM 9", L"NUM *", L"NUM +", L"", L"NUM -", L"NUM .", L"NUM /",
        L"F1", L"F2", L"F3", L"F4", L"F5", L"F6", L"F7", L"F8", L"F9", L"F10", L"F11", L"F12", L"F13", L"F14", L"F15", L"F16",
        L"F17", L"F18", L"F19", L"F20", L"F21", L"F22", L"F23", L"F24", L"", L"", L"", L"", L"", L"", L"", L"",
        L"Num Lock", L"Scroll Lock"
    };

    void ProcessInputMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
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

    set<uint_fast8_t> GetPressedKeys() {
        return set<uint_fast8_t>(pressedKeys);
    }

    set<uint_fast8_t> GetPressedKeyboardKeys() {
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

    set<uint_fast8_t> GetPressedMouseKeys() {
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

    wstring GetReadableKeyString(const set<uint_fast8_t>& keys) {
        set<uint_fast8_t> remaining(keys);
        wstringstream ws;
        if (remaining.find(VK_CONTROL) != remaining.end()) {
            ws << L"Ctrl + ";
            remaining.erase(VK_CONTROL);
        }
        if (remaining.find(VK_MENU) != remaining.end()) {
            ws << L"Alt + ";
            remaining.erase(VK_MENU);
        }
        if (remaining.find(VK_SHIFT) != remaining.end()) {
            ws << L"Shift + ";
            remaining.erase(VK_SHIFT);
        }
        bool doneFirst = false;
        for (const auto k : remaining) {
            auto str = vkStrings[k];
            if (!str[0]) {
                if (doneFirst) {
                    ws << L" + ";
                }

                // Empty key, try getting the actual translation from Windows
                UINT code = MapVirtualKey(k, MAPVK_VK_TO_VSC);
                if (code) {
                    wchar_t chr[8];
                    GetKeyNameText(code << 16, chr, 8);
                    ws << chr;
                }
            } else {
                if (doneFirst) {
                    ws << L" + ";
                }
                ws << str;
            }
            doneFirst = true;
        }

        return ws.str();
    }

}
