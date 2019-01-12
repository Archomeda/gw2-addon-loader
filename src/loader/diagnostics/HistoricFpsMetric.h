#pragma once
#pragma once
#include "../stdafx.h"
#include "TimeMetric.h"
#include "HistoricMetric.h"

namespace loader::diagnostics {

    class HistoricFpsMetric : TimeMetric {
    public:
        HistoricFpsMetric() { }
        HistoricFpsMetric(int maxHistorySize) {
            this->historicMetric = HistoricMetric<float>(maxHistorySize);
        }

        TimeMetric::Prepare;
        TimeMetric::Start;
        TimeMetric::Stop;

        TimeMetric::Continue;
        TimeMetric::Pause;

        void MarkFrame() { this->Stop(); this->Start(); }

        int GetMaxHistorySize() const { return this->historicMetric.GetMaxHistorySize(); }

        float GetLast() const { return this->historicMetric.GetLast(); }
        float GetMovingAverage() const { return this->historicMetric.GetMovingAverage(); }
        float GetMovingMaximum() const { return this->historicMetric.GetMovingMaximum(); }
        float GetOverallMaximum() const { return this->historicMetric.GetOverallMaximum(); }

        const std::vector<float> GetMovingHistory() const { return this->historicMetric.GetMovingHistory(); }

    protected:
        virtual void Add(intmax_t nanoseconds) override { this->historicMetric.Add(10000000000 / nanoseconds / 10.0f); }

    private:
        HistoricMetric<float> historicMetric;
    };
}
