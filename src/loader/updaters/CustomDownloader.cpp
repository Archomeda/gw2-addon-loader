#include "CustomDownloader.h"

using namespace std;

#define BUFFER_SIZE 4096

namespace loader::updaters {

    void CustomDownloader::DownloadUpdate() {
        this->downloadBuffer = new char[BUFFER_SIZE];
        auto result = this->downloadFunc(this, this->downloadBuffer, BUFFER_SIZE, &CustomDownloader::WriteBuffer);
        delete[] this->downloadBuffer;

        if (!result) {
            this->DownloadComplete(this, this->data, this->error);
        }
        else {
            this->error = "Failed with error " + result;
        }
        this->CleanUpDownload();
    }

    void CustomDownloader::WriteBuffer(void* downloader, size_t bytesWritten, size_t totalSize) {
        CustomDownloader* obj = static_cast<CustomDownloader*>(downloader);
        obj->dataSize = totalSize;
        if (bytesWritten == 0) {
            return;
        }

        // Boundary corrections
        if (bytesWritten > BUFFER_SIZE) {
            bytesWritten = BUFFER_SIZE;
        }

        lock_guard<mutex> guard(obj->dataMutex);
        obj->data.insert(obj->data.end(), obj->downloadBuffer, obj->downloadBuffer + bytesWritten);
        obj->dataProgress += bytesWritten;
        obj->ProgressUpdate(obj, obj->dataProgress, obj->dataSize);
    }

}
