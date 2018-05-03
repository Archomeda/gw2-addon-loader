#pragma once
#include "../../stdafx.h"
#include "../../addons/Addon.h"

namespace loader {
    namespace gui {
        namespace elements {

            bool AddonListItem(const addons::Addon* addon, bool selected = false, const ImVec2& size = ImVec2(0, 0));
            bool AddonListItem(const addons::Addon* addon, bool* selected, const ImVec2& size = ImVec2(0, 0));

        }
    }
}
