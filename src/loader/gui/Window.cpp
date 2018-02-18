#include "Window.h"
#include "gui_manager.h"

namespace loader {
    namespace gui {

        void Window::Show() {
            ShowWindow(shared_from_this());
        }
        
        void Window::Close() {
            CloseWindow(shared_from_this());
        }

        bool Window::IsOpen() {
            return IsWindowOpen(shared_from_this());
        }

    }
}
