#pragma once
#include "../stdafx.h"
#include "TimeMetric.h"

namespace loader::diagnostics {

    class SingleTimeMetric : TimeMetric {
    public:
        TimeMetric::Prepare;
        TimeMetric::Start;
        TimeMetric::Stop;

        TimeMetric::Continue;
        TimeMetric::Pause;

        intmax_t GetTime() const { return this->time; }
        const std::string GetTimeFormatted() const { return this->ToFormattedTimeString(this->time); }

    protected:
        virtual void Add(intmax_t nanoseconds) override { this->time = nanoseconds; }

    private:
        intmax_t time = 0;
    };
}
