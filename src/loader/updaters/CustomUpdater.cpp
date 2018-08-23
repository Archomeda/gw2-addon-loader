#include "CustomUpdater.h"
#include "../log.h"
#include "../utils/string.h"

using namespace std;
using namespace loader::utils;

#define VERSION_SIZE 32
#define INFOURL_SIZE 512
#define DOWNLOADURL_SIZE 512

namespace loader::updaters {

    VersionInfo CustomUpdater::CheckLatestVersion() {
        VersionInfo versionInfo = {};

        UPDATERS_LOG()->info("Start checking for updates by calling external function for add-on");
        UpdateCheckDetails update;
        update.versionSize = VERSION_SIZE;
        update.infoUrlSize = INFOURL_SIZE;
        update.downloadUrlSize = DOWNLOADURL_SIZE;
        update.version = new char[VERSION_SIZE];
        update.infoUrl = new char[INFOURL_SIZE];
        update.downloadUrl = new char[DOWNLOADURL_SIZE];
        if (!this->checkUpdateFunc(&update)) {
            if (update.version) {
                versionInfo.version = cstr2str(update.version, update.versionSize);
            }
            if (update.infoUrl) {
                versionInfo.infoUrl = cstr2str(update.infoUrl, update.infoUrlSize);
            }
            if (update.downloadUrl) {
                versionInfo.downloadUrl = cstr2str(update.downloadUrl, update.downloadUrlSize);
            }
        }
        delete[] update.version;
        delete[] update.infoUrl;
        delete[] update.downloadUrl;

        if (versionInfo.version.empty()) {
            UPDATERS_LOG()->info("Add-on did not return a version");
            return {};
        }
        UPDATERS_LOG()->info("Latest update {0}: {1} - {2}", versionInfo.version, versionInfo.infoUrl, versionInfo.downloadUrl);
        return versionInfo;
    }

}
