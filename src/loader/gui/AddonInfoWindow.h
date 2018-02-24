#pragma once
#include "Window.h"

namespace loader {
    namespace gui {

        class AddonInfoWindow : public Window {
        public:
            AddonInfoWindow();

            void SetSelectedAddon(int selectedAddon) { this->selectedAddon = selectedAddon; }

            void Render() override;

        protected:
            int selectedAddon = -1;
        };
    
    }
}
