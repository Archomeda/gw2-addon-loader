#pragma once
#include "Window.h"

namespace loader {
    namespace gui {

        class MessageWindow : public Window {
        public:
            MessageWindow(const std::string& title, const std::string& message);

            void Render() override;

            static std::shared_ptr<MessageWindow> ShowMessageWindow(const std::string& title, const std::string& message);

        protected:
            const std::string Message;

        };

    }
}
