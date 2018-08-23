#include "HttpDownloader.h"
#include "../utils/encoding.h"

using namespace std;
using namespace loader::utils;

namespace loader::updaters {

    void HttpDownloader::DownloadUpdate() {
        string url = this->url;
        HINTERNET hSession = InternetOpen(L"Guild Wars 2 Add-on Loader", 0, NULL, NULL, 0);
        if (hSession == NULL) {
            this->error = "Failed to open session";
            this->FinishDownload();
            this->DownloadComplete(this, {}, this->error);
            this->CleanUpDownload();
            return;
        }

        HINTERNET hOpenUrl = InternetOpenUrl(hSession, u16(url).c_str(), NULL, 0, 1, 1);
        if (hOpenUrl == NULL) {
            InternetCloseHandle(hSession);
            this->error = "Failed to open URL";
            this->FinishDownload();
            this->DownloadComplete(this, {}, this->error);
            this->CleanUpDownload();
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
                if (bytesRead == 0) {
                    break;
                }
                lock_guard<mutex> guard(this->dataMutex);
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

        this->FinishDownload();
        this->DownloadComplete(this, this->data, this->error);
        this->CleanUpDownload();
    }
}
