#pragma once
#include "../stdafx.h"
#include "Window.h"
#include "../addons/addon.h"

namespace loader::gui {

    class DisclaimerWindow : public Window {
    public:
        DisclaimerWindow();

        void Render() override;
    };

}
