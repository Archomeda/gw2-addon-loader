#pragma once
#include "stdafx.h"

namespace loader::imgui {

    void ProcessInputMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    bool RepeatedPressedKeys();
    const std::set<uint_fast8_t> GetPressedKeys();
    const std::set<uint_fast8_t> GetPressedMouseKeys();
    const std::set<uint_fast8_t> GetPressedKeyboardKeys();

    bool DoKeysContainNonModifiers(const std::set<uint_fast8_t>& keys);
    const std::string GetReadableKeyString(const std::set<uint_fast8_t>& keys);

}
