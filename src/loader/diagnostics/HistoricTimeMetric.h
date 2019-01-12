#pragma once
#include "../stdafx.h"
#include "TimeMetric.h"
#include "HistoricMetric.h"

namespace loader::diagnostics {

    template<std::intmax_t TimeUnit, int Decimals>
    class HistoricTimeMetric : TimeMetric {
    public:
        HistoricTimeMetric() { }
        HistoricTimeMetric(int maxHistorySize) {
            this->historicMetric = HistoricMetric<float>(maxHistorySize);
        }

        TimeMetric::Prepare;

        void Start() {
            if (this->isStarted) {
                return;
            }

            TimeMetric::Start();
            ++this->calls;
        }

        TimeMetric::Stop;

        void Continue() {
            if (!this->isStarted || !this->isPaused) {
                return;
            }

            TimeMetric::Continue();
            ++this->calls;
        }

        TimeMetric::Pause;

        uint64_t GetCalls() const { return this->calls; }

        int GetMaxHistorySize() const { return this->historicMetric.GetMaxHistorySize(); }

        float GetLast() const { return this->historicMetric.GetLast(); }
        const std::string GetLastTimeFormatted() const {
            return this->ToFormattedTimeString(static_cast<intmax_t>(this->historicMetric.GetLast() * this->timePrecisionRatio * this->decimalPrecisionRatio));
        }

        float GetMovingAverage() const { return this->historicMetric.GetMovingAverage(); }
        const std::string GetMovingAverageFormatted() const {
            return this->ToFormattedTimeString(static_cast<intmax_t>(this->historicMetric.GetMovingAverage() * this->timePrecisionRatio * this->decimalPrecisionRatio));
        }

        float GetMovingMaximum() const { return this->historicMetric.GetMovingMaximum(); }
        const std::string GetMovingMaximumFormatted() const {
            return this->ToFormattedTimeString(static_cast<intmax_t>(this->historicMetric.GetMovingMaximum() * this->timePrecisionRatio * this->decimalPrecisionRatio));
        }

        float GetOverallMaximum() const { return this->historicMetric.GetOverallMaximum(); }
        const std::string GetOverallMaximumFormatted() const {
            return this->ToFormattedTimeString(static_cast<intmax_t>(this->historicMetric.GetOverallMaximum() * this->timePrecisionRatio * this->decimalPrecisionRatio));
        }

        const std::vector<float> GetMovingHistory() const { return this->historicMetric.GetMovingHistory(); }

    protected:
        virtual void Add(intmax_t nanoseconds) override {
            this->historicMetric.Add(static_cast<int>(nanoseconds / this->timePrecisionRatio) / static_cast<float>(this->decimalPrecisionRatio));
        }

    private:
        uint64_t calls = 0;
        HistoricMetric<float> historicMetric;

        template<typename B, typename E>
        std::intmax_t constexpr pow(B base, E exponent) {
            static_assert(std::is_integral<E>(), "Exponent must be integral");
            return exponent == 0 ? 1 : base * pow(base, exponent - 1);
        }
        const std::intmax_t decimalPrecisionRatio = pow(10, Decimals);
        const std::intmax_t timePrecisionRatio = 1000000000 / TimeUnit / this->decimalPrecisionRatio;
    };

}
