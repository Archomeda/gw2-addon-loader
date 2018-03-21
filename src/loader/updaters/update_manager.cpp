#include "update_manager.h"
#include <chrono>
#include "../Config.h"
#include "../log.h"
#include "../version.h"
#include "../addons/addons_manager.h"

using namespace std;
using namespace loader::addons;
using namespace loader::updaters;

namespace loader {
    namespace updaters {

        GithubReleasesUpdater LoaderUpdater("archomeda/gw2-addon-loader");
        shared_ptr<Installer> LoaderUpdaterInstaller;
        map<Addon*, shared_ptr<Installer>> AddonUpdateInstallers;

        void CheckUpdates() {
            using namespace chrono;
            auto lastCheckAgo = (time_point_cast<seconds>(system_clock::now()).time_since_epoch() - AppConfig.GetLastUpdateCheck().time_since_epoch()).count();
            if (lastCheckAgo < 24 * 60 * 60) {
                GetLog()->info("Skipped checking for updates, last check was {0} seconds ago", to_string(lastCheckAgo));
                return;
            }

            LoaderUpdater.SetCheckCallback([](const Updater* const updater, VersionInfo version) {
                if (AppConfig.GetLastestVersion() != version.version) {
                    GetLog()->info("New Addon Loader version found: {0}", version.version);
                    AppConfig.SetLastestVersion(version.version);
                    AppConfig.SetLastestVersionInfoUrl(version.infoUrl);
                    AppConfig.SetLastestVersionDownloadUrl(version.downloadUrl);
                }
                AppConfig.SetLastUpdateCheck(chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()));
            });
            LoaderUpdater.CheckForUpdateAsync();

            for (const auto& addon : AddonsList) {
                if (addon->SupportsUpdating()) {
                    addon->CheckUpdate([&addon](const Updater* const updater, VersionInfo version) {
                        if (addon->GetVersion() != version.version) {
                            GetLog()->info("New version found for {0}: {1}", addon->GetName(), version.version);
                            AppConfig.SetLastestAddonVersion(addon.get(), version.version);
                            AppConfig.SetLastestAddonVersionInfoUrl(addon.get(), version.infoUrl);
                            AppConfig.SetLastestAddonVersionDownloadUrl(addon.get(), version.downloadUrl);
                        }
                    });
                }
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

            LoaderUpdaterInstaller = make_shared<Installer>(versionInfo);
            LoaderUpdaterInstaller->StartInstall();
        }

        void InstallUpdate(Addon* const addon) {
            VersionInfo versionInfo = addon->GetLatestVersion();
            if (versionInfo.downloadUrl.empty()) {
                return;
            }

            shared_ptr<Installer> installer = make_shared<Installer>(versionInfo, addon);
            AddonUpdateInstallers[addon] = installer;
            installer->StartInstall();
        }

    }
}
