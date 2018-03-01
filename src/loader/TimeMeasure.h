#pragma once
#include <chrono>
#include <vector>

namespace loader {
    
    class TimeMeasure {
    public:
        static const int Frames = 4 * 60;

        TimeMeasure();

        void StartFrame();
        void EndFrame();
        void StartMeasurement();
        void EndMeasurement();

        float GetLast() const { return this->movingHistory.at(this->movingHistory.size() - 1); }
        float GetOverallMaximum() const { return this->overallMaximum; }
        float GetMovingAverage() const { return this->movingAverage; }
        const std::vector<float> GetMovingHistory() const { return this->movingHistory; }
        uint64_t GetCalls() const { return this->calls; }

    private:
        std::chrono::steady_clock::time_point measureStart;
        float currentMeasure;

        float overallMaximum;
        float movingAverage;
        std::vector<float> movingHistory;

        uint64_t calls;

    };

}
