#pragma once
#include <memory>

namespace loader {
    namespace gui {

        class Window;

        void ShowWindow(const std::shared_ptr<Window>& window);

        void CloseWindow(const std::shared_ptr<Window>& window);

        bool IsWindowOpen(const std::shared_ptr<Window>& window);

        void Render();

    }
}
