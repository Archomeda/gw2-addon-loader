#pragma once
#include "../stdafx.h"
#include "Downloader.h"
#include "Updater.h"
#include "../addons/Addon.h"

namespace loader::updaters {

    class Installer {
    public:
        Installer(const VersionInfo version);
        Installer(const VersionInfo version, const std::shared_ptr<addons::Addon> addon);

        void StartInstall();

        bool IsActive() const { return this->active; }
        bool HasCompleted() const { return this->completed; }
        float GetProgressFraction() const { return this->progressFraction; }
        const std::string GetDetailedProgress();

    private:
        void DownloaderProgressUpdate(const Downloader* const downloader, std::size_t progress, std::size_t total);
        void DownloaderComplete(const Downloader* const downloader, const std::vector<char>& data, const std::string& errorMessage);
        void Extract(const std::vector<char>& data);
        void WriteFile(const std::vector<char>& fileData, const std::experimental::filesystem::path& fileName);

        void SetDetailedProgress(const std::string& progress);

        std::shared_ptr<addons::Addon> addon;

        VersionInfo version;
        std::unique_ptr<Downloader> downloader;
        std::string targetSubfolder;
        std::string targetFileName;

        std::atomic_bool active = false;
        std::atomic_bool completed = false;
        std::atomic<float> progressFraction = 0;
        std::mutex detailedProgressMutex;
        std::string detailedProgress;
    };

}
