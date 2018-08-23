#include "Updater.h"

using namespace std;

namespace loader::updaters {
    Updater::~Updater() {
        if (this->updateTask.joinable()) {
            this->updateTask.join();
        }
    }

    void Updater::CheckForUpdateAsync() {
        if (!this->active) {
            this->active = true;
            this->updateTask = thread([](Updater* updater) {
                SetThreadDescription(GetCurrentThread(), L"[LOADER] Add-on Update Checker");
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

                VersionInfo version = updater->CheckLatestVersion();
                updater->latestVersion = version;
                if (updater->checkCallback != nullptr) {
                    updater->checkCallback(updater, version);
                }
                updater->active = false;
            }, this);
        }
    }

}
