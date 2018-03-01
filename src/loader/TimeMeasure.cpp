#include "TimeMeasure.h"

using namespace std;

namespace loader {

    TimeMeasure::TimeMeasure() {
        this->movingHistory.resize(this->Frames);
        this->movingHistory.reserve(2 * this->Frames);
    }

    void TimeMeasure::StartFrame() {
        this->currentMeasure = 0;
    }

    void TimeMeasure::EndFrame() {
        this->movingHistory.erase(this->movingHistory.begin());
        this->movingHistory.push_back(this->currentMeasure);

        if (this->currentMeasure > this->overallMaximum) {
            this->overallMaximum = this->currentMeasure;
        }
        this->movingAverage = this->movingAverage * 0.97f + this->currentMeasure * 0.03f;
    }

    void TimeMeasure::StartMeasurement() {
        this->measureStart = chrono::steady_clock::now();
    }

    void TimeMeasure::EndMeasurement() {
        this->currentMeasure += (chrono::steady_clock::now() - this->measureStart).count() / 1000 / 1000.0f; // Precise to a microsecond
        ++this->calls;
    }

}
