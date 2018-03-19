#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <vector>

namespace loader {
    namespace updaters {

        class Downloader;

        typedef void(ProgressUpdateCallback_t)(const Downloader* const downloader, std::size_t progress, std::size_t total);
        typedef void(DownloadCompleteCallback_t)(const Downloader* const downloader, const std::vector<char>& data, const std::string& errorMessage);

        class Downloader {
        public:
            Downloader() = default;
            Downloader(const std::string& url) : url(url) { }

            void StartDownloadAsync();

            void SetProgressUpdateCallback(std::function<ProgressUpdateCallback_t> callback) { this->progressCallback = callback; }
            void SetDownloadCompleteCallback(std::function<DownloadCompleteCallback_t> callback) { this->completeCallback = callback; }

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

            std::function<ProgressUpdateCallback_t> progressCallback;
            std::function<DownloadCompleteCallback_t> completeCallback;
        };

    }
}
