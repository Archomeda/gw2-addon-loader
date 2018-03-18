#pragma once
#include <memory>
#include "GithubReleasesUpdater.h"
#include "Installer.h"

namespace loader {
    namespace updaters {

        extern GithubReleasesUpdater LoaderUpdater;
        extern std::shared_ptr<Installer> LoaderUpdaterInstaller;

        void CheckUpdates();
        void InstallUpdate();
        void LoaderUpdateCheckCallback(const Updater* const updater, VersionInfo version);

    }
}
