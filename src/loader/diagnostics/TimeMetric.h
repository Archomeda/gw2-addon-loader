#pragma once
#include "../stdafx.h"

namespace loader::diagnostics {

    class TimeMetric {
    public:
        virtual void Prepare();
        virtual void Start();
        virtual void Stop();

        virtual void Continue();
        virtual void Pause();

    protected:
        virtual const std::string ToFormattedTimeString(intmax_t nanoseconds) const;
        virtual void Add(intmax_t nanoseconds) = 0;

        bool isStarted = false;
        bool isPaused = false;
        std::chrono::steady_clock::time_point measureStart;
        intmax_t currentMeasure = 0;
    };

}
