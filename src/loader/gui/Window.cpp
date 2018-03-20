#include "Window.h"
#include "gui_manager.h"

namespace loader {
    namespace gui {

        void Window::Show() {
            ShowWindow(this);
        }

        void Window::Close() {
            CloseWindow(this);
        }

        bool Window::IsOpen() {
            return IsWindowOpen(this);
        }

    }
}
