#pragma once
#include <string>
#include <vector>

namespace loader {
    namespace utils {

        std::vector<char> DownloadFromUrl(const std::string& url);

    }
}
