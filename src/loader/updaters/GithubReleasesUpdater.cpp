#include "GithubReleasesUpdater.h"
#include "../globals.h"
#include "../log.h"
#include "../utils/web.h"

using namespace std;
using json = nlohmann::json;
using namespace loader::utils;

namespace loader::updaters {

    VersionInfo GithubReleasesUpdater::CheckLatestVersion() {
        VersionInfo versionInfo = {};

        UPDATERS_LOG()->info("Start checking for updates on GitHub repository {0}", this->repository);
        const auto data = DownloadFromUrl(string(this->githubApiBaseUrl) + "/repos/" + this->repository + "/releases");
        if (data.size() == 0) {
            UPDATERS_LOG()->warn("Checking for updates on GitHub repository {0} failed: empty response", this->repository);
            return versionInfo;
        }

        auto doc = json::parse(data.begin(), data.end());
        try {
            for (const auto& release : doc) {
                if (release.find("tag_name") == release.end()) {
                    continue;
                }
                if (release.find("prerelease") != release.end() && release["prerelease"].get<bool>()) {
                    // Prerelease, skipping for now
                    continue;
                }
                versionInfo.version = release["tag_name"].get<string>();
                versionInfo.infoUrl = release["html_url"].get<string>();
                if (release.find("assets") != release.end()) {
                    for (const auto& asset : release["assets"]) {
                        if (asset.find("name") == asset.end() || asset.find("browser_download_url") == asset.end()) {
                            continue;
                        }
                        string assetName = asset["name"].get<string>();
                        regex re(".*" PLATFORM ".*\\.(zip|dll)");
                        smatch match;
                        if (regex_search(assetName, match, re) && match.size() > 1) {
                            versionInfo.downloadUrl = asset["browser_download_url"].get<string>();
                            break;
                        }
                    }
                }
                break;
            }
        }
        catch (const exception& ex) {
            UPDATERS_LOG()->warn("Parsing JSON result from GitHub releases API for repository {0} failed: {1}", this->repository, ex.what());
            return versionInfo;
        }

        if (versionInfo.version.empty()) {
            UPDATERS_LOG()->info("No update info found on GitHub repository {0}", this->repository);
        }
        else {
            UPDATERS_LOG()->info("Latest update on GitHub repository {0}: {1} - {2} ({3})", this->repository, versionInfo.version, versionInfo.infoUrl, versionInfo.downloadUrl);
        }
        return versionInfo;
    }

}
