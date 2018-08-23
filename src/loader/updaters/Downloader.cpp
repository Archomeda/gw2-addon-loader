#include "Downloader.h"

using namespace std;

namespace loader::updaters {

    void Downloader::StartDownloadAsync() {
        if (!this->busy) {
            this->busy = true;
            this->ProgressUpdate(this, 0, 0);
            this->downloadTask = async(launch::async, &Downloader::DownloadUpdate, this);
        }
    }

    void Downloader::FinishDownload() {
        if (this->busy) {
            this->busy = false;
            this->completed = this->data.size() == this->dataSize || (this->data.size() > 0 && this->dataSize == 0);
        }
    }

    void Downloader::CleanUpDownload() {
        this->downloadTask = {};
    }

    const vector<char> Downloader::GetData() {
        lock_guard<mutex> guard(this->dataMutex);
        return this->data;
    }

}
