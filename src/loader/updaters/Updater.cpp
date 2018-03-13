#include "Updater.h"

using namespace std;

namespace loader {
    namespace updaters {

        void Updater::CheckForUpdateAsync() {
            if (!this->updateTask.valid()) {
                this->updateTask = async(launch::async, [](Updater* updater) {
                    UpdatedVersion version = updater->CheckLatestVersion();
                    updater->latestVersion = version.version;
                    updater->latestVersionInfoUrl = version.infoUrl;
                    if (updater->callback != nullptr) {
                        updater->callback(updater);
                    }
                    updater->updateTask = {};
                }, this);
            }
        }

    }
}
