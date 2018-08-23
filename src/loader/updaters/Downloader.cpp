#include "Downloader.h"

using namespace std;

namespace loader::updaters {
    Downloader::~Downloader() {
        if (this->downloadTask.joinable()) {
            this->downloadTask.join();
        }
    }

    void Downloader::StartDownloadAsync() {
        if (!this->active) {
            this->active = true;
            this->ProgressUpdate(this, 0, 0);
            this->downloadTask = thread([](Downloader* downloader) {
                SetThreadDescription(GetCurrentThread(), L"[LOADER] Add-on Updater");
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

                downloader->DownloadUpdate();

                downloader->active = false;
                downloader->completed = downloader->data.size() == downloader->dataSize || (downloader->data.size() > 0 && downloader->dataSize == 0);
                downloader->DownloadComplete(downloader, downloader->data, downloader->error);
            }, this);
        }
    }

    const vector<char> Downloader::GetData() {
        lock_guard<mutex> guard(this->dataMutex);
        return this->data;
    }

}
