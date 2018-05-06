#pragma once
#include "../../stdafx.h"

namespace loader {
    namespace gui {
        namespace elements {

            bool KeybindBox(const char* label, std::set<uint_fast8_t>& keys, bool* selected, const ImVec2& size = ImVec2(0, 0));

        }
    }
}