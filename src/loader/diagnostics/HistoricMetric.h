#pragma once
#include "../stdafx.h"

namespace loader::diagnostics {

    template<typename T>
    class HistoricMetric {
    public:
        HistoricMetric() : HistoricMetric(4 * 60) { }
        HistoricMetric(int maxHistorySize) : maxHistorySize(maxHistorySize) {
            this->movingHistory.resize(maxHistorySize);
            this->movingHistory.reserve(2 * this->maxHistorySize);
        }

        void Add(T value) {
            this->last = value;

            if (this->movingHistory.size() == this->maxHistorySize) {
                this->movingHistory.erase(this->movingHistory.begin());
            }
            this->movingHistory.push_back(value);
            --this->movingMaximumIndex;

            // Determine the new moving maximum by checking if the newer value is greater than our current one
            if (value > this->movingMaximum) {
                this->movingMaximum = value;
                this->movingMaximumIndex = this->maxHistorySize;
            }
            // If the moving maximum fell off the grid, determine the new moving maximum from the moving history
            if (this->movingMaximumIndex < 0) {
                this->movingMaximumIndex = static_cast<int>(std::distance(this->movingHistory.begin(), std::max_element(this->movingHistory.begin(), this->movingHistory.end())));
                this->movingMaximum = this->movingHistory.at(this->movingMaximumIndex);
            }

            // Overall maximum
            if (value > this->overallMaximum) {
                this->overallMaximum = value;
            }

            // Moving average
            this->movingAverage = (this->movingAverage * 0.97f) + (value * 0.03f);
        }

        int GetMaxHistorySize() const { return this->maxHistorySize; }

        T GetLast() const { return this->last; }
        T GetMovingAverage() const { return this->movingAverage; }
        T GetMovingMaximum() const { return this->movingMaximum; }
        T GetOverallMaximum() const { return this->overallMaximum; }
        const std::vector<T> GetMovingHistory() const { return this->movingHistory; }

    private:
        int maxHistorySize = 4 * 60;
        int movingMaximumIndex = 0;

        uint64_t calls = 0;
        T last = 0;
        T movingAverage = 0;
        T movingMaximum = 0;
        T overallMaximum = 0;
        std::vector<T> movingHistory;
    };

}
