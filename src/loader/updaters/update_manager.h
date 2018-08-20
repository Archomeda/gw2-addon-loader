#pragma once
#include "../stdafx.h"
#include "GithubReleasesUpdater.h"
#include "Installer.h"
#include "../addons/Addon.h"

namespace loader::updaters {

    extern GithubReleasesUpdater LoaderUpdater;
    extern std::shared_ptr<Installer> LoaderUpdaterInstaller;
    extern std::map<addons::Addon*, std::shared_ptr<Installer>> AddonUpdateInstallers;

    void CheckUpdates();
    void InstallUpdate();
    void InstallUpdate(addons::Addon* const addon);

}
