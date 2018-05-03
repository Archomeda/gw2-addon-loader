#pragma once
#include "../stdafx.h"

namespace loader {
    namespace addons {

        enum AddonMetricType {
            FrameBasedMetric,
            SingleMetric
        };

        class AddonMetric {
        public:
            static const int Frames = 4 * 60;

            AddonMetric(AddonMetricType metricType = AddonMetricType::FrameBasedMetric);

            AddonMetricType GetMetricType() const { return this->metricType; }
            void SetMetricType(AddonMetricType metricType) { this->metricType = metricType; }

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
            AddonMetricType metricType;

            std::chrono::steady_clock::time_point measureStart;
            float currentMeasure = 0;

            float overallMaximum = 0;
            float movingAverage = 0;
            std::vector<float> movingHistory;

            uint64_t calls = 0;
        };

    }
}
