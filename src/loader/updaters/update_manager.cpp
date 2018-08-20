#include "update_manager.h"
#include "../Config.h"
#include "../log.h"
#include "../version.h"
#include "../addons/addons_manager.h"

using namespace std;
using namespace loader::addons;
using namespace loader::updaters;

namespace loader::updaters {

    GithubReleasesUpdater LoaderUpdater("archomeda/gw2-addon-loader");
    shared_ptr<Installer> LoaderUpdaterInstaller;
    map<Addon*, shared_ptr<Installer>> AddonUpdateInstallers;

    void CheckUpdates() {
        using namespace chrono;
        auto lastCheckAgo = (time_point_cast<seconds>(system_clock::now()).time_since_epoch() - AppConfig.GetLastUpdateCheck().time_since_epoch()).count();
        if (lastCheckAgo < 24 * 60 * 60) {
            UPDATERS_LOG()->info("Skipped checking for updates, last check was {0} seconds ago", to_string(lastCheckAgo));
            return;
        }

        UPDATERS_LOG()->info("Checking for updates");
        LoaderUpdater.SetCheckCallback([](const Updater* const updater, VersionInfo version) {
            if (AppConfig.GetLatestVersion() != version.version) {
                UPDATERS_LOG()->info("New Add-on Loader version found: {0}", version.version);
                AppConfig.SetLatestVersion(version.version);
                AppConfig.SetLatestVersionInfoUrl(version.infoUrl);
                AppConfig.SetLatestVersionDownloadUrl(version.downloadUrl);
            }
            AppConfig.SetLastUpdateCheck(chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()));
        });
        LoaderUpdater.CheckForUpdateAsync();

        for (const auto& addon : Addons) {
            if (addon->SupportsUpdating()) {
                addon->CheckUpdate([&addon](const Updater* const updater, VersionInfo version) {
                    if (addon->GetVersion() != version.version) {
                        UPDATERS_LOG()->info("New version found for {0}: {1}", addon->GetName(), version.version);
                        AppConfig.SetLatestAddonVersion(addon.get(), version.version);
                        AppConfig.SetLatestAddonVersionInfoUrl(addon.get(), version.infoUrl);
                        AppConfig.SetLatestAddonVersionDownloadUrl(addon.get(), version.downloadUrl);
                    }
                });
            }
        }
    }

    void InstallUpdate() {
        VersionInfo versionInfo;
        versionInfo.version = AppConfig.GetLatestVersion();
        versionInfo.infoUrl = AppConfig.GetLatestVersionInfoUrl();
        versionInfo.downloadUrl = AppConfig.GetLatestVersionDownloadUrl();
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
