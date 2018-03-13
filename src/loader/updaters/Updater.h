#pragma once
#include <future>
#include <string>

namespace loader {
    namespace updaters {

        class Updater;

        struct UpdatedVersion {
            std::string version;
            std::string infoUrl;
        };

        typedef void(*UpdateCheckCallback_t)(const Updater* updater);

        class Updater {
        public:
            void SetCallback(UpdateCheckCallback_t callback) { this->callback = callback; }

            void CheckForUpdateAsync();
            const std::string GetLatestVersion() const { return this->latestVersion; }
            const std::string GetLatestVersionInfoUrl() const { return this->latestVersionInfoUrl; }

        protected:
            virtual UpdatedVersion CheckLatestVersion() = 0;

        private:
            std::future<void> updateTask;
            std::string latestVersion;
            std::string latestVersionInfoUrl;
            UpdateCheckCallback_t callback;

        };

    }
}
