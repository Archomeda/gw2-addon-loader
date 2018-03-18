#include "GithubReleasesUpdater.h"
#include <rapidjson/document.h>
#include <miniz.h>
#include <regex>
#include "../globals.h"
#include "../log.h"
#include "../utils/web.h"

using namespace std;
using namespace rapidjson;
using namespace loader::utils;

namespace loader {
    namespace updaters {

        VersionInfo GithubReleasesUpdater::CheckLatestVersion() {
            VersionInfo versionInfo = {};

            GetLog()->info("Start checking for updates on GitHub repository {0}", this->repository);
            const auto data = DownloadFromUrl(string(this->githubApiBaseUrl) + "/repos/" + this->repository + "/releases");
            if (data.size() == 0) {
                GetLog()->warn("Checking for updates on GitHub repository {0} failed: empty response", this->repository);
                return versionInfo;
            }

            string jsonString(data.begin(), data.end());
            Document json;
            json.Parse<0>(jsonString.c_str());
            if (!json.IsArray()) {
                GetLog()->warn("Checking for updates on GitHub repository {0} failed: invalid response", this->repository);
                return versionInfo;
            }

            for (SizeType i = 0; i < json.Size(); ++i) {
                if (!json[i].HasMember("tag_name") || !json[i]["tag_name"].IsString()) {
                    continue;
                }
                if (json[i].HasMember("prerelease") && json[i]["prerelease"].IsBool() && json[i]["prerelease"].GetBool()) {
                    // Prerelease, skipping for now
                    continue;
                }
                versionInfo.version = json[i]["tag_name"].GetString();
                versionInfo.infoUrl = json[i]["html_url"].GetString();
                if (json[i].HasMember("assets") && json[i]["assets"].IsArray()) {
                    const auto& assets = json[i]["assets"];
                    for (SizeType j = 0; j < assets.Size(); ++j) {
                        if (!assets[j].HasMember("name") || !assets[j]["name"].IsString() ||
                            !assets[j].HasMember("browser_download_url") || !assets[j]["browser_download_url"].IsString()) {
                            continue;
                        }
                        string assetName = assets[j]["name"].GetString();
                        regex re(".*" PLATFORM ".*\\.(zip|dll)");
                        smatch match;
                        if (regex_search(assetName, match, re) && match.size() > 1) {
                            versionInfo.downloadUrl = assets[j]["browser_download_url"].GetString();
                            break;
                        }
                    }
                }
                break;
            }

            if (versionInfo.version == "") {
                GetLog()->info("No update info found on GitHub repository {0}", this->repository);
            }
            else {
                GetLog()->info("Latest update on GitHub repository {0}: {1} - {2} ({3})", this->repository, versionInfo.version, versionInfo.infoUrl, versionInfo.downloadUrl);
            }
            return versionInfo;
        }

    }
}
