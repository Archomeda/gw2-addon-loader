#pragma once
#include "../stdafx.h"
#include "Downloader.h"

namespace loader::updaters {

    class Updater;

    struct VersionInfo {
        std::string version;
        std::string infoUrl;
        std::string downloadUrl;
    };

    typedef void(UpdateCheckCallback_t)(const Updater* const updater, const VersionInfo version);

    class Updater {
    public:
        ~Updater();

        void SetCheckCallback(std::function<UpdateCheckCallback_t> callback) { this->checkCallback = callback; }

        void CheckForUpdateAsync();

        const VersionInfo GetLatestVersion() const { return this->latestVersion; }

    protected:
        virtual VersionInfo CheckLatestVersion() = 0;

    private:
        std::atomic_bool active = false;
        std::thread updateTask;
        VersionInfo latestVersion;
        std::function<UpdateCheckCallback_t> checkCallback;
    };

}
