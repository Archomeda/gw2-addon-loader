#pragma once
#include "Window.h"

namespace loader {
    namespace gui {

        class MessageWindow : public Window {
        public:
            MessageWindow(const std::wstring& title, const std::wstring& message);

            void Render() override;

            static std::shared_ptr<MessageWindow> ShowMessageWindow(const std::wstring& title, const std::wstring& message);

        protected:
            const std::wstring Message;

        };

    }
}
