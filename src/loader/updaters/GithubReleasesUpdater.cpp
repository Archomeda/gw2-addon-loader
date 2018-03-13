#include "GithubReleasesUpdater.h"
#include <rapidjson/document.h>
#include "../log.h"
#include "../utils/web.h"

using namespace std;
using namespace rapidjson;
using namespace loader::utils;

namespace loader {
    namespace updaters {

        UpdatedVersion GithubReleasesUpdater::CheckLatestVersion() {
            UpdatedVersion updatedVersion = {};

            GetLog()->info("Start checking for updates on GitHub repository {0}", this->repository);
            auto data = DownloadFromUrl(string(this->githubApiBaseUrl) + "/repos/" + this->repository + "/releases");
            if (data.size() == 0) {
                GetLog()->warn("Checking for updates on GitHub repository {0} failed: empty response", this->repository);
                return updatedVersion;
            }

            string jsonString(data.begin(), data.end());
            Document json;
            json.Parse<0>(jsonString.c_str());
            if (!json.IsArray()) {
                GetLog()->warn("Checking for updates on GitHub repository {0} failed: invalid response", this->repository);
                return updatedVersion;
            }

            for (SizeType i = 0; i < json.Size(); ++i) {
                if (!json[i].HasMember("tag_name") || !json[i]["tag_name"].IsString()) {
                    continue;
                }
                if (json[i].HasMember("prerelease") && json[i]["prerelease"].IsBool() && json[i]["prerelease"].GetBool()) {
                    // Prerelease, skipping for now
                    continue;
                }
                updatedVersion.version = json[i]["tag_name"].GetString();
                updatedVersion.infoUrl = json[i]["html_url"].GetString();
                break;
            }

            if (updatedVersion.version == "") {
                GetLog()->info("No update info found on GitHub repository {0}", this->repository);
            }
            else {
                GetLog()->info("Latest update on GitHub repository {0}: {1} - {2}", this->repository, updatedVersion.version, updatedVersion.infoUrl);
            }
            return updatedVersion;
        }

    }
}
