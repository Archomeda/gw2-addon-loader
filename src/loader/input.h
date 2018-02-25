#pragma once
#include <set>
#include <stdint.h>
#include <string>
#include "windows.h"

namespace loader {

    void ProcessInputMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    bool RepeatedPressedKeys();
    std::set<uint_fast8_t> GetPressedKeys();
    std::set<uint_fast8_t> GetPressedMouseKeys();
    std::set<uint_fast8_t> GetPressedKeyboardKeys();

    bool DoKeysContainNonModifiers(const std::set<uint_fast8_t>& keys);
    std::wstring GetReadableKeyString(const std::set<uint_fast8_t>& keys);

}
