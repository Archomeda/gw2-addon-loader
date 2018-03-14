#pragma once
#include <chrono>

namespace loader {

    class TimeDuration {
    public:
        void Start();
        void End();

        float GetDuration() const { return this->duration; }

    private:
        std::chrono::steady_clock::time_point start;
        float duration;
    };

}
