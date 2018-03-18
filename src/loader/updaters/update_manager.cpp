#include "update_manager.h"
#include <chrono>
#include "../Config.h"
#include "../log.h"

using namespace std;

namespace loader {
    namespace updaters {

        GithubReleasesUpdater LoaderUpdater("archomeda/gw2-addon-loader");
        shared_ptr<Installer> LoaderUpdaterInstaller;

        void CheckUpdates() {
            using namespace chrono;
            auto lastCheckAgo = (time_point_cast<seconds>(system_clock::now()).time_since_epoch() - AppConfig.GetLastUpdateCheck().time_since_epoch()).count();
            if (lastCheckAgo > 24 * 60 * 60) {
                GetLog()->info("Checking for Addon Loader updates");
                LoaderUpdater.SetCheckCallback(&LoaderUpdateCheckCallback);
                LoaderUpdater.CheckForUpdateAsync();
            }
            else {
                GetLog()->info("Skipped checking for Addon Loader updates, last check was {0} seconds ago", to_string(lastCheckAgo));
            }
        }

        void InstallUpdate() {
            VersionInfo versionInfo;
            versionInfo.version = AppConfig.GetLastestVersion();
            versionInfo.infoUrl = AppConfig.GetLastestVersionInfoUrl();
            versionInfo.downloadUrl = AppConfig.GetLastestVersionDownloadUrl();
            if (versionInfo.downloadUrl.empty()) {
                return;
            }

            LoaderUpdaterInstaller = shared_ptr<Installer>(new Installer(versionInfo));
            LoaderUpdaterInstaller->StartInstall();
        }

        void LoaderUpdateCheckCallback(const Updater* const updater, VersionInfo version) {
            using namespace chrono;
            AppConfig.SetLastestVersion(version.version);
            AppConfig.SetLastestVersionInfoUrl(version.infoUrl);
            AppConfig.SetLastestVersionDownloadUrl(version.downloadUrl);
            AppConfig.SetLastUpdateCheck(time_point_cast<seconds>(system_clock::now()));
        }

    }
}
