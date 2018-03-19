#include "Updater.h"

using namespace std;

namespace loader {
    namespace updaters {

        void Updater::CheckForUpdateAsync() {
            if (!this->updateTask.valid()) {
                this->updateTask = async(launch::async, [](Updater* updater) {
                    VersionInfo version = updater->CheckLatestVersion();
                    updater->latestVersion = version;
                    if (updater->checkCallback != nullptr) {
                        updater->checkCallback(updater, version);
                    }
                    updater->updateTask = {};
                }, this);
            }
        }

    }
}
