#pragma once
#include "Window.h"
#include "../addons/addon.h"

namespace loader {
    namespace gui {

        class AddonInfoWindow : public Window {
        public:
            AddonInfoWindow();

            void SetAddon(const addons::Addon* const addon) { this->addon = addon; }

            void Render() override;

        protected:
            const addons::Addon* addon = nullptr;
        };

    }
}
