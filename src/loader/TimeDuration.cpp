#include "TimeDuration.h"

using namespace std;

namespace loader {

    void TimeDuration::Start() {
        this->start = chrono::steady_clock::now();
    }

    void TimeDuration::End() {
        this->duration += (chrono::steady_clock::now() - this->start).count() / 1000.0f; // Precise to a nanosecond
    }

}
