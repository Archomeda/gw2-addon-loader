#pragma once
#include <memory>
#include <vector>
#include "Addon.h"

namespace loader {
    namespace addons {

        extern std::vector<std::shared_ptr<Addon>> AddonsList;

        void RefreshAddonList();

    }
}