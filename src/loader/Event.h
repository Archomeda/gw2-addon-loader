#pragma once
#include "stdafx.h"

namespace loader {

    template<typename... Args>
    class Event final {
    public:
        using call_type = void(Args...);

        struct EventListenerDelegate {
            EventListenerDelegate(Event* const listener, const std::string& id) : listener(listener), id(id) { };

            void operator+=(const std::function<call_type>& delegate) {
                listener->Add(this->id, delegate);
            }
            
            Event* const listener;
            const std::string id;
        };

        Event() = default;
        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        EventListenerDelegate operator[](const std::string& id) {
            return EventListenerDelegate(this, id);
        }

        void operator+=(const std::function<call_type>& delegate) {
            this->Add(delegate);
        }

        void operator-=(const std::string& id) {
            this->Remove(id);
        }

        void operator()(Args... args) const {
            this->Raise(args...);
        }

        void Add(const std::string& id, const std::function<call_type>& delegate) {
            this->listeners[id] = delegate;
        }

        void Add(const std::function<call_type>& delegate) {
            this->anonymousListeners.push_back(delegate);
        }

        void Remove(const std::string& id) {
            this->listeners.erase(this->listeners.find(id));
        }

        void Raise(Args... args) const {
            for (const auto& listener : this->listeners) {
                listener.second(args...);
            }
            for (const auto& listener : this->anonymousListeners) {
                listener(args...);
            }
        }

    private:
        std::map<std::string, std::function<call_type>> listeners;
        std::vector<std::function<call_type>> anonymousListeners;

    };

}
