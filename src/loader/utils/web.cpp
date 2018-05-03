#include "web.h"
#include "encoding.h"

using namespace std;

namespace loader {
    namespace utils {

        const vector<char> DownloadFromUrl(const string& url) {
            vector<char> data;
            HINTERNET hSession = InternetOpen(L"Guild Wars 2 Addon Loader", 0, NULL, NULL, 0);
            if (hSession == NULL) {
                // Failed to open session
                return data;
            }

            HINTERNET hOpenUrl = InternetOpenUrl(hSession, u16(url).c_str(), NULL, 0, 1, 1);
            if (hOpenUrl == NULL) {
                // Failed to open URL
                InternetCloseHandle(hSession);
                return data;
            }

            char* buffer = new char[4096];
            DWORD bytesRead = 0;
            while (true) {
                if (InternetReadFile(hOpenUrl, buffer, 4096, &bytesRead)) {
                    if (bytesRead == 0) {
                        break;
                    }
                    data.insert(data.end(), buffer, buffer + bytesRead);
                }
                else {
                    // Failed to read from URL
                    break;
                }
            }

            delete[] buffer;
            InternetCloseHandle(hOpenUrl);
            InternetCloseHandle(hSession);
            return data;
        }

    }
}
