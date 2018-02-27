#include "encoding.h"
#include <codecvt>

using namespace std;

namespace loader {
    namespace utils {

        string u8(const wstring& wstr) {
            return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wstr);
        }

        wstring u16(const string& str) {
            return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str);
        }

    }
}