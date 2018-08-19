#include "AddonMetric.h"

using namespace std;

namespace loader {
    namespace addons {

        AddonMetric::AddonMetric(AddonMetricType metricType) : metricType(metricType) {
            this->movingHistory.resize(this->Frames);
            this->movingHistory.reserve(2 * this->Frames);
        }

        void AddonMetric::StartFrame() {
            this->currentMeasure = 0;
        }

        void AddonMetric::EndFrame() {
            this->movingHistory.erase(this->movingHistory.begin());
            this->movingHistory.push_back(this->currentMeasure);

            if (this->currentMeasure > this->overallMaximum) {
                this->overallMaximum = this->currentMeasure;
            }
            this->movingAverage = this->movingAverage * 0.97f + this->currentMeasure * 0.03f;
        }

        void AddonMetric::StartMeasurement() {
            if (this->metricType != AddonMetricType::FrameBasedMetric) {
                this->StartFrame();
            }
            this->measureStart = chrono::steady_clock::now();
        }

        void AddonMetric::EndMeasurement() {
            this->currentMeasure += (chrono::steady_clock::now() - this->measureStart).count() / 1000.0f; // Precise to a nanosecond
            ++this->calls;
            if (this->metricType != AddonMetricType::FrameBasedMetric) {
                this->EndFrame();
            }
        }

        const string AddonMetric::FormatTimeMetric(float microseconds) const {
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
}
