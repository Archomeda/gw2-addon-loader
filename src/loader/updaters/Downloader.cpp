#include "Downloader.h"
#include "../utils/encoding.h"

using namespace std;
using namespace loader::utils;

namespace loader::updaters {

    void Downloader::StartDownloadAsync() {
        if (!this->busy) {
            this->busy = true;
            this->ProgressUpdate(this, 0, 0);

            this->downloadTask = async(launch::async, [=]() {

                string url = this->url;
                HINTERNET hSession = InternetOpen(L"Guild Wars 2 Add-on Loader", 0, NULL, NULL, 0);
                if (hSession == NULL) {
                    this->error = "Failed to open session";
                    this->busy = false;
                    this->DownloadComplete(this, {}, this->error);
                    return;
                }

                HINTERNET hOpenUrl = InternetOpenUrl(hSession, u16(url).c_str(), NULL, 0, 1, 1);
                if (hOpenUrl == NULL) {
                    InternetCloseHandle(hSession);
                    this->error = "Failed to open URL";
                    this->busy = false;
                    this->DownloadComplete(this, {}, this->error);
                    return;
                }

                DWORD contentLength;
                DWORD contentLengthSize = sizeof(contentLength);
                if (HttpQueryInfo(hOpenUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, static_cast<LPVOID>(&contentLength), &contentLengthSize, NULL)) {
                    this->dataSize = contentLength;
                    this->data.reserve(contentLength);
                }

                char* buffer = new char[4096];
                DWORD bytesRead = 0;
                while (true) {
                    if (InternetReadFile(hOpenUrl, buffer, 4096, &bytesRead)) {
                        lock_guard<mutex> guard(this->dataMutex);
                        if (bytesRead == 0) {
                            break;
                        }
                        this->data.insert(this->data.end(), buffer, buffer + bytesRead);
                        this->dataProgress += bytesRead;
                        this->ProgressUpdate(this, this->dataProgress, this->dataSize);
                    }
                    else {
                        this->error = "Failed to read from URL";
                        break;
                    }
                }

                delete[] buffer;
                InternetCloseHandle(hOpenUrl);
                InternetCloseHandle(hSession);

                this->busy = false;
                this->completed = this->data.size() == this->dataSize || (this->data.size() > 0 && this->dataSize == 0);
                this->DownloadComplete(this, this->data, this->error);
                this->downloadTask = {};

            });
        }

    }

    const vector<char> Downloader::GetData() {
        lock_guard<mutex> guard(this->dataMutex);
        return this->data;
    }

}
