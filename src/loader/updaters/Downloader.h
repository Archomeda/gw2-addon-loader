#pragma once
#include "../stdafx.h"
#include "../Event.h"

namespace loader::updaters {

    class Downloader;

    typedef void(ProgressUpdateCallback_t)(const Downloader* const downloader, std::size_t progress, std::size_t total);
    typedef void(DownloadCompleteCallback_t)(const Downloader* const downloader, const std::vector<char>& data, const std::string& errorMessage);

    class Downloader {
    public:
        Event<const Downloader* const, std::size_t, std::size_t> ProgressUpdate;
        Event<const Downloader* const, const std::vector<char>&, const std::string&> DownloadComplete;

        void StartDownloadAsync();

        std::size_t GetTotalSize() const { return this->dataSize; }
        std::size_t GetProgress() const { return this->dataProgress; }
        const std::vector<char> GetData();

        virtual bool IsValid() const = 0;
        bool IsBusy() const { return this->busy; }
        bool HasCompleted() const { return this->completed; }
        bool HasError() const { return !this->error.empty(); }
        std::string GetErrorMessage() const { return this->error; }

    protected:
        virtual void DownloadUpdate() = 0;
        void FinishDownload();
        void CleanUpDownload();
        
        std::vector<char> data;
        std::mutex dataMutex;
        std::atomic_size_t dataSize = 0;
        std::atomic_size_t dataProgress = 0;

        std::string error;

    private:
        std::future<void> downloadTask;
        std::atomic_bool busy = false;
        std::atomic_bool completed = false;
    };

}
