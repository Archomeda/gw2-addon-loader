#pragma once
#include "../../stdafx.h"

namespace loader::gui::elements {

    bool KeybindBox(const char* label, uint_fast8_t* key, bool* selected, const ImVec2& size = ImVec2(0, 0));
    bool KeybindBox(const char* label, std::set<uint_fast8_t>& keys, bool* selected, const ImVec2& size = ImVec2(0, 0));

}
