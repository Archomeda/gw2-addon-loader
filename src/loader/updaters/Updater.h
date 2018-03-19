#pragma once
#include <functional>
#include <future>
#include <memory>
#include <string>
#include "Downloader.h"

namespace loader {
    namespace updaters {

        class Updater;

        struct VersionInfo {
            std::string version;
            std::string infoUrl;
            std::string downloadUrl;
        };

        typedef void(UpdateCheckCallback_t)(const Updater* const updater, const VersionInfo version);

        class Updater {
        public:
            void SetCheckCallback(std::function<UpdateCheckCallback_t> callback) { this->checkCallback = callback; }

            void CheckForUpdateAsync();

            const VersionInfo GetLatestVersion() const { return this->latestVersion; }

        protected:
            virtual VersionInfo CheckLatestVersion() = 0;

        private:
            std::future<void> updateTask;
            VersionInfo latestVersion;
            std::function<UpdateCheckCallback_t> checkCallback;
        };

    }
}
