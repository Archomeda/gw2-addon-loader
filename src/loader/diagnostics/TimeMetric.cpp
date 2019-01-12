#include "TimeMetric.h"

using namespace std;

namespace loader::diagnostics {

    void TimeMetric::Prepare() {
        if (this->isStarted) {
            return;
        }

        this->isStarted = true;
        this->isPaused = true;
        this->currentMeasure = 0;
        this->measureStart = { };
    }

    void TimeMetric::Start() {
        if (this->isStarted) {
            return;
        }

        this->isStarted = true;
        this->isPaused = false;
        this->currentMeasure = 0;
        this->measureStart = chrono::steady_clock::now();
    }

    void TimeMetric::Stop() {
        if (!this->isStarted) {
            return;
        }

        this->isStarted = false;
        this->isPaused = false;
        if (this->measureStart.time_since_epoch().count() > 0) {
            this->currentMeasure += (chrono::steady_clock::now() - this->measureStart).count();
        }
        if (this->currentMeasure > 0) {
            this->Add(this->currentMeasure);
        }
        this->measureStart = { };
    }

    void TimeMetric::Continue() {
        if (!this->isStarted || !this->isPaused) {
            return;
        }

        this->isPaused = false;
        this->measureStart = chrono::steady_clock::now();
    }

    void TimeMetric::Pause() {
        if (!this->isStarted || this->isPaused) {
            return;
        }

        this->isPaused = true;
        this->currentMeasure += (chrono::steady_clock::now() - this->measureStart).count();
        this->measureStart = { };
    }

    const string TimeMetric::ToFormattedTimeString(intmax_t nanoseconds) const {
        float microseconds = static_cast<float>(nanoseconds / 1000);
        stringstream ss;
        ss.imbue(locale(""));

        if (microseconds < 1000) {
            ss << fixed << setprecision(0) << microseconds << " µs";
            return ss.str();
        }
        float milliseconds = microseconds / 1000;
        if (milliseconds < 1000) {
            ss << fixed << setprecision(2) << milliseconds << " ms";
            return ss.str();
        }
        float seconds = milliseconds / 1000;
        ss << fixed << setprecision(2) << seconds << " s";
        return ss.str();
    }

}
