#pragma once
#include <atomic>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include "Updater.h"
#include "../addons/Addon.h"

namespace loader {
    namespace updaters {

        class Installer {
        public:
            Installer(const VersionInfo version);
            Installer(const VersionInfo version, const addons::Addon* addon);

            void StartInstall();

            bool IsBusy() const { return this->busy; }
            bool HasCompleted() const { return this->completed; }
            const std::string GetDetailedProgress();

        private:
            void DownloaderProgressUpdate(const Downloader* const downloader, std::size_t progress, std::size_t total);
            void DownloaderComplete(const Downloader* const downloader, const std::vector<char>& data, const std::string& errorMessage);
            void Extract(const std::vector<char>& data);
            void WriteFile(const std::vector<char>& fileData, const std::experimental::filesystem::path& fileName);

            void SetDetailedProgress(const std::string& progress);

            VersionInfo version;
            std::unique_ptr<Downloader> downloader;
            std::string targetSubfolder;
            std::string targetFileName;

            std::atomic_bool busy = false;
            std::atomic_bool completed = false;
            std::mutex detailedProgressMutex;
            std::string detailedProgress;

            std::future<void> extractTask;
        };

    }
}
