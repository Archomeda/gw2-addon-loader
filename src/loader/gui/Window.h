#pragma once
#include <memory>
#include <string>
#include <imgui.h>

namespace loader {
    namespace gui {

        enum WindowType {
            GenericWindow,
            ModalWindow
        };

        class Window : public std::enable_shared_from_this<Window> {
        public:
            Window() { }

            virtual void Render() = 0;

            virtual void BeginStyle() { }
            virtual void EndStyle() { }

            virtual void Show();
            virtual void Close();
            virtual bool IsOpen();

            const std::wstring GetTitle() const { return this->title; }
            void SetTitle(const std::wstring& title) { this->title = title; }
            WindowType GetType() const { return this->type; }
            void SetType(WindowType type) { this->type = type; }
            ImGuiWindowFlags GetFlags() const { return this->flags; }
            void SetFlags(ImGuiWindowFlags flags) { this->flags = flags; }

        private:
            std::wstring title;
            WindowType type = WindowType::GenericWindow;
            ImGuiWindowFlags flags;
        };

    }
}
