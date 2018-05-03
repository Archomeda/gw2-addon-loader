#pragma once
#include "../stdafx.h"
#include "../Event.h"

namespace loader {
    namespace updaters {

        class Downloader;

        typedef void(ProgressUpdateCallback_t)(const Downloader* const downloader, std::size_t progress, std::size_t total);
        typedef void(DownloadCompleteCallback_t)(const Downloader* const downloader, const std::vector<char>& data, const std::string& errorMessage);

        class Downloader {
        public:
            Downloader() = default;
            Downloader(const std::string& url) : url(url) { }

            Event<const Downloader* const, std::size_t, std::size_t> ProgressUpdate;
            Event<const Downloader* const, const std::vector<char>&, const std::string&> DownloadComplete;

            void StartDownloadAsync();

            const std::string GetUrl() const { return this->url; }
            std::size_t GetTotalSize() const { return this->dataSize; }
            std::size_t GetProgress() const { return this->dataProgress; }
            const std::vector<char> GetData();

            bool IsValid() const { return !this->url.empty(); }
            bool IsBusy() const { return this->busy; }
            bool HasCompleted() const { return this->completed; }
            bool HasError() const { return !this->error.empty(); }
            std::string GetErrorMessage() const { return this->error; }

        private:
            std::future<void> downloadTask;
            std::string url;
            std::vector<char> data;
            std::mutex dataMutex;
            std::atomic_size_t dataSize = 0;
            std::atomic_size_t dataProgress = 0;

            std::atomic_bool busy = false;
            std::atomic_bool completed = false;
            std::string error;
        };

    }
}
