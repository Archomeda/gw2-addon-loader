#pragma once
#include "Updater.h"

namespace loader {
    namespace updaters {

        class GithubReleasesUpdater : public Updater {
        public:
            GithubReleasesUpdater(const std::string& repository) : repository(repository) { }

        protected:
            virtual VersionInfo CheckLatestVersion() override;

        private:
            const char* githubApiBaseUrl = "https://api.github.com";
            std::string repository;
        };

    }
}
